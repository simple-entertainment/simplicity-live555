#include "ResourceSink.h"

using namespace std;

namespace simplicity
{
	namespace live555
	{
		ResourceSink::ResourceSink(Resource* resource) :
				resource(resource),
				startCode()
		{
			startCode.push_back(0x00);
			startCode.push_back(0x00);
			startCode.push_back(0x01);
		}


		void ResourceSink::onFrameReceived(const unsigned char* rtpFrameData, unsigned int rtpFrameSize,
				unsigned int numTruncatedBytes, struct timeval presentationTime,
				unsigned int durationInMicroseconds)
		{
			// The RTP frame received here contains one h246 NAL unit (without a start code).

			resource->appendData(startCode.data(), startCode.size());
			resource->appendData(reinterpret_cast<const char*>(rtpFrameData), rtpFrameSize);
		}
	}
}
