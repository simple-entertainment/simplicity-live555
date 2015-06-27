#ifndef SOURCE_H_
#define SOURCE_H_

#include <string>

#include <liveMedia.hh>

#include "SourceFrame.h"

namespace simplicity
{
	namespace live555
	{
		class Source
		{
			public:
				class InternalSource : public FramedSource
				{
					public:
						InternalSource(UsageEnvironment& environment, Source& delegate);

						void doGetNextFrame();

					private:
						Source& delegate;
				};

				class InternalSubSession : public OnDemandServerMediaSubsession
				{
					public:
						InternalSubSession(UsageEnvironment& environment, Source& delegate);

						void checkForAuxSDPLine();

						FramedSource* createNewStreamSource(unsigned int /* clientSessionId */,
								unsigned int& /* estimatedBitrate */);

						RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char /* rtpPayloadTypeIfDynamic */,
								FramedSource* /* inputSource */);

						char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);

					private:
						std::string auxSDPLine;

						Source& delegate;

						RTPSink* rtpSink;

						char stopAuxSDPLooping;
				};

				Source();

				OnDemandServerMediaSubsession* getInternalSubSession();

				virtual bool getNextFrame(SourceFrame& sourceFrame) = 0;

				void setEnvironment(UsageEnvironment& environment);

			private:
				InternalSubSession* internalSubSession;
		};
	}
}

#endif /* SOURCE_H_ */
