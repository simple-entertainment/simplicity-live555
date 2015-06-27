#ifndef TEXTURESINK_H_
#define TEXTURESINK_H_

#include <simplicity/rendering/Texture.h>

#include "Sink.h"

namespace simplicity
{
	namespace live555
	{
		class SIMPLE_API TextureSink : public Sink
		{
			public:
				TextureSink(Texture& texture);

				virtual ~TextureSink();

			private:
				// FFMPEG pollutes the global namespace (being C) and conflicts with simplicity symbols.
				// To workaround this I have used a void pointer here which I populate with a type defined in the
				// implementation file. This allows me to move the FFMPEG headers to the implementation file.
				void* codec;

				std::vector<unsigned char> h264Frame;

				unsigned int h264FramePosition;

				double previousNormalPlayTime;

				Texture* texture;

				bool addedSpsPpsNalUnits;

				void addStartCode();

				void onFrameReceived(const unsigned char* rtpFrameData, unsigned int rtpFrameSize,
						unsigned int numTruncatedBytes, struct timeval presentationTime,
						unsigned int durationInMicroseconds);

				void initConversion();
		};
	}
}

#endif /* TEXTURESINK_H_ */
