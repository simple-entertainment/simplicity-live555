#ifndef SINK_H_
#define SINK_H_

#include <vector>

#include <liveMedia.hh>

#include <simplicity/common/Defines.h>

namespace simplicity
{
	namespace live555
	{
		class SIMPLE_API Sink
		{
			public:
				class InternalSink : public MediaSink
				{
					public:
						InternalSink(UsageEnvironment& environment, Sink& delegate);

						void onFrameReceived(unsigned int rtprameSize, unsigned int numTruncatedBytes,
								struct timeval presentationTime, unsigned int durationInMicroseconds);

					private:
						Sink& delegate;

						std::vector<unsigned char> rtpFrame;

						Boolean continuePlaying() override;
				};

				Sink();

				virtual ~Sink();

				MediaSink* getInternalSink();

				MediaSubsession* getSubSession();

				void setEnvironment(UsageEnvironment& environment);

				void setSubSession(MediaSubsession* subSession);

			private:
				InternalSink* internalSink;

				MediaSubsession* subSession;

				virtual void onFrameReceived(const unsigned char* rtpFrameData, unsigned int rtpFrameSize,
						unsigned int numTruncatedBytes, struct timeval presentationTime,
						unsigned int durationInMicroseconds) = 0;
		};
	}
}

#endif /* SINK_H_ */
