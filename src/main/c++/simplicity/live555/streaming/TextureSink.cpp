extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#include <simplicity/logging/Logs.h>

#include "TextureSink.h"

using namespace std;

const int FRAME_SIZE = 1000000;

namespace simplicity
{
	namespace live555
	{
		struct TextureSinkCodec
		{
			AVCodec* codec = nullptr;

			AVCodecContext* context = nullptr;

			SwsContext* conversionContext = nullptr;

			AVFrame* convertedFrame = nullptr;

			AVFrame* decodedFrame = nullptr;

			AVPacket packet;
		};

		TextureSink::TextureSink(Texture& texture) :
			addedSpsPpsNalUnits(false),
			codec(new TextureSinkCodec),
			h264Frame(FRAME_SIZE),
			h264FramePosition(0),
			previousNormalPlayTime(0.0),
			texture(&texture)
		{
			TextureSinkCodec* codec = static_cast<TextureSinkCodec*>(this->codec);

			avcodec_register_all();

			codec->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
			if (codec->codec == nullptr)
			{
				Logs::error("simplicity::live555", "Codec not found");
			}

			codec->context = avcodec_alloc_context3(codec->codec);
			if (codec->context == nullptr)
			{
				Logs::error("simplicity::live555", "Could not allocate video codec context");
			}

			if (codec->codec->capabilities & CODEC_CAP_TRUNCATED)
				codec->context->flags |= CODEC_FLAG_TRUNCATED;

			//we can receive truncated frames
			codec->context->flags2 |= CODEC_FLAG2_CHUNKS;

			if (avcodec_open2(codec->context, codec->codec, nullptr) < 0)
			{
				Logs::error("simplicity::live555", "Could not open codec");
			}

			av_init_packet(&codec->packet);

			codec->decodedFrame = av_frame_alloc();
		}

		TextureSink::~TextureSink()
		{
			TextureSinkCodec* codec = static_cast<TextureSinkCodec*>(this->codec);

			av_frame_free(&codec->convertedFrame);
			av_frame_free(&codec->decodedFrame);
			avcodec_close(codec->context);
			av_free(codec->context);

			delete codec;
		}

		void TextureSink::addStartCode()
		{
			h264Frame[h264FramePosition++] = 0x00;
			h264Frame[h264FramePosition++] = 0x00;
			h264Frame[h264FramePosition++] = 0x01;
		}

		void TextureSink::onFrameReceived(const unsigned char* rtpFrameData, unsigned int rtpFrameSize,
				unsigned int numTruncatedBytes, struct timeval presentationTime, unsigned int durationInMicroseconds)
		{
			// The RTP frame received here contains one h246 NAL unit (without a start code).

			h264FramePosition = 0;

			if (!addedSpsPpsNalUnits)
			{
				unsigned int sPropRecordCount = 0;
				SPropRecord* sPropRecords =
						parseSPropParameterSets(getSubSession()->fmtp_spropparametersets(), sPropRecordCount);
				for (unsigned int index = 0; index < sPropRecordCount; index++)
				{
					addStartCode();
					memcpy(&h264Frame[h264FramePosition], sPropRecords[index].sPropBytes,
							sPropRecords[index].sPropLength);
					h264FramePosition += sPropRecords[index].sPropLength;
				}
				delete[] sPropRecords;
				addedSpsPpsNalUnits = true; // for next time
			}

			addStartCode();
			memcpy(&h264Frame[h264FramePosition], rtpFrameData, rtpFrameSize);
			h264FramePosition += rtpFrameSize;

			TextureSinkCodec* codec = static_cast<TextureSinkCodec*>(this->codec);

			codec->packet.size = h264FramePosition;
			codec->packet.data = h264Frame.data();

			int got_picture_ptr = 0;
			if (avcodec_decode_video2(codec->context, codec->decodedFrame, &got_picture_ptr,
					&codec->packet) < 0)
			{
				Logs::warning("simplicity::live555", "Failed to decode frame");
			}

			if (got_picture_ptr > 0)
			{
				if (codec->conversionContext == nullptr)
				{
					initConversion();
				}

				sws_scale(codec->conversionContext, codec->decodedFrame->data, codec->decodedFrame->linesize,
						0, codec->context->height, codec->convertedFrame->data,
						codec->convertedFrame->linesize);

				texture->setRawData(reinterpret_cast<char*>(codec->convertedFrame->data[0]));
			}
		}

		void TextureSink::initConversion()
		{
			TextureSinkCodec* codec = static_cast<TextureSinkCodec*>(this->codec);

			codec->conversionContext = sws_getContext(codec->context->width, codec->context->height,
					codec->context->pix_fmt, codec->context->width, codec->context->height, AV_PIX_FMT_RGB24,
					SWS_BICUBIC, nullptr, nullptr, nullptr);

			codec->convertedFrame = av_frame_alloc();
			// TODO Are we leaking the uint8_t array we allocate here?
			avpicture_alloc(reinterpret_cast<AVPicture*>(codec->convertedFrame), AV_PIX_FMT_RGB24,
					codec->context->width, codec->context->height);
		}
	}
}
