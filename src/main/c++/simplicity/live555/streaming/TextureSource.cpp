extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <simplicity/logging/Logs.h>
#include <simplicity/resources/Resources.h>

#include "TextureSource.h"

void ffmpeg_dump_yuv(char *filename, AVPicture *pic, int width, int height)
{
    FILE *fp =0;
    int i,j,shift;

    fp = fopen(filename,"wb");
    if(fp) {
        for(i = 0; i < 3; i++) {
            shift = (i == 0 ? 0:1);
            uint8_t* yuv_factor = pic->data[i];
            for(j = 0; j < (height>>shift); j++) {
                fwrite(yuv_factor,(width>>shift),1,fp);
                yuv_factor += pic->linesize[i];
            }
        }
        fclose(fp);

    }
}

namespace simplicity
{
	namespace live555
	{
		unsigned int frameNum = 0;

		struct TextureSourceCodec
		{
			AVCodec* codec = nullptr;

			AVCodecContext* context = nullptr;

			SwsContext* conversionContext = nullptr;

			AVFrame* convertedFrame = nullptr;

			AVPacket packet;

			AVFrame* rawFrame = nullptr;
		};

		TextureSource::TextureSource(const Texture& texture) :
				codec(new TextureSourceCodec),
				texture(&texture)
		{
			TextureSourceCodec* codec = static_cast<TextureSourceCodec*>(this->codec);

			avcodec_register_all();

			codec->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (codec->codec == nullptr)
			{
				Logs::error("simplicity::live555", "Codec not found");
			}

			codec->context = avcodec_alloc_context3(codec->codec);
			if (codec->context == nullptr)
			{
				Logs::error("simplicity::live555", "Could not allocate video codec context");
			}

			/* put sample parameters */
			codec->context->bit_rate = 400000;
			/* resolution must be a multiple of two */
			codec->context->width = 1024;
			codec->context->height = 768;
			/* frames per second */
			codec->context->time_base = (AVRational){1,60};
			/* emit one intra frame every ten frames
			 * check frame pict_type before passing frame
			 * to encoder, if codec->rawFrame->pict_type is AV_PICTURE_TYPE_I
			 * then gop_size is ignored and the output of encoder
			 * will always be I frame irrespective to gop_size
			 */
			codec->context->gop_size = 10;
			codec->context->max_b_frames = 1;
			codec->context->pix_fmt = AV_PIX_FMT_YUV420P;

			// TODO Use av_dict_set instead?
			av_opt_set(codec->context->priv_data, "preset", "ultrafast", 0);
			av_opt_set(codec->context->priv_data, "tune", "zerolatency", 0);

			if (avcodec_open2(codec->context, codec->codec, nullptr) < 0)
			{
				Logs::error("simplicity::live555", "Could not open codec");
			}

			codec->rawFrame = av_frame_alloc();
			codec->rawFrame->format = AV_PIX_FMT_RGB24;
			codec->rawFrame->width  = codec->context->width;
			codec->rawFrame->height = codec->context->height;

			/* the image can be allocated by any means and av_image_alloc() is
			 * just the most convenient way if av_malloc() is to be used */
			if (av_image_alloc(codec->rawFrame->data, codec->rawFrame->linesize, codec->context->width,
					codec->context->height, AV_PIX_FMT_RGB24, 16) < 0)
			{
				Logs::error("simplicity::live555", "Could not allocate raw picture buffer");
			}

			codec->convertedFrame = av_frame_alloc();
			codec->convertedFrame->format = codec->context->pix_fmt;
			codec->convertedFrame->width  = codec->context->width;
			codec->convertedFrame->height = codec->context->height;

			if (av_image_alloc(codec->convertedFrame->data, codec->convertedFrame->linesize, codec->context->width,
					codec->context->height, codec->context->pix_fmt, 32) < 0)
			{
				Logs::error("simplicity::live555", "Could not allocate raw picture buffer");
			}
		}

		TextureSource::~TextureSource()
		{
			TextureSourceCodec* codec = static_cast<TextureSourceCodec*>(this->codec);

			avcodec_close(codec->context);
			av_free(codec->context);
			av_freep(&codec->rawFrame->data[0]);
			av_frame_free(&codec->rawFrame);
			av_frame_free(&codec->convertedFrame);
		}

		bool TextureSource::getNextFrame(SourceFrame& sourceFrame)
		{
			TextureSourceCodec* codec = static_cast<TextureSourceCodec*>(this->codec);

			av_init_packet(&codec->packet);
			codec->packet.data = nullptr;
			codec->packet.size = 0;

			const char* rgbData = texture->getRawData();
			memcpy(codec->rawFrame->data[0], rgbData, codec->context->width * codec->context->height * 3);

			if (codec->conversionContext == nullptr)
			{
				initConversion();
			}

			sws_scale(codec->conversionContext, codec->rawFrame->data, codec->rawFrame->linesize,
					0, codec->context->height, codec->convertedFrame->data,
					codec->convertedFrame->linesize);

			/* prepare a dummy image */
			/* Y */
			/*for (unsigned int y = 0; y < codec->context->height; y++) {
				for (unsigned int x = 0; x < codec->context->width; x++) {
					codec->convertedFrame->data[0][y * codec->convertedFrame->linesize[0] + x] = x + y + frameNum * 3;
				}
			}*/

			/* Cb and Cr */
			/*for (unsigned int y = 0; y < codec->context->height/2; y++) {
				for (unsigned int x = 0; x < codec->context->width/2; x++) {
					codec->convertedFrame->data[1][y * codec->convertedFrame->linesize[1] + x] = 128 + y + frameNum * 2;
					codec->convertedFrame->data[2][y * codec->convertedFrame->linesize[2] + x] = 64 + x + frameNum * 5;
				}
			}
			frameNum++;*/

			// TODO?
			codec->rawFrame->pts = frameNum;

			/*ffmpeg_dump_yuv("test.yuv", reinterpret_cast<AVPicture*>(codec->convertedFrame), codec->context->width,
					codec->context->height);*/

			int got_packet_ptr = 0;
			if (avcodec_encode_video2(codec->context, &codec->packet, codec->convertedFrame, &got_packet_ptr) < 0)
			{
				Logs::warning("simplicity::live555", "Failed to encode frame");
			}

			Resources::get("test.264", Category::UNCATEGORIZED, true)->
					appendData(reinterpret_cast<char*>(codec->packet.data), codec->packet.size);

			if (got_packet_ptr > 0)
			{
				gettimeofday(&sourceFrame.presentationTime, nullptr);
				sourceFrame.frameSize = codec->packet.size;
				memcpy(sourceFrame.frameData, codec->packet.data, codec->packet.size);
				av_free_packet(&codec->packet);
			}

			return true;
		}

		void TextureSource::initConversion()
		{
			TextureSourceCodec* codec = static_cast<TextureSourceCodec*>(this->codec);

			codec->conversionContext = sws_getContext(codec->context->width, codec->context->height,
					AV_PIX_FMT_RGB24, codec->context->width, codec->context->height, codec->context->pix_fmt,
					SWS_BICUBIC, nullptr, nullptr, nullptr);
		}
	}
}
