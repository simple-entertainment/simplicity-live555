#ifndef RESOURCESINK_H_
#define RESOURCESINK_H_

#include <vector>

#include <simplicity/common/Defines.h>
#include <simplicity/resources/Resource.h>

#include "Sink.h"

namespace simplicity
{
	namespace live555
	{
		class SIMPLE_API ResourceSink : public Sink
		{
			public:
				ResourceSink(Resource* resource);

			private:
				Resource* resource;

				std::vector<char> startCode;

				void onFrameReceived(const unsigned char* rtpFrameData, unsigned int rtpFrameSize,
						unsigned int numTruncatedBytes, struct timeval presentationTime,
						unsigned int durationInMicroseconds);
		};
	}
}

#endif /* RESOURCESINK_H_ */
