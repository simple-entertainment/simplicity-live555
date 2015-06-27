#include "Source.h"

namespace simplicity
{
	namespace live555
	{
		void checkForAuxSDPLine(void* clientData)
		{
			Source::InternalSubSession* subSession = static_cast<Source::InternalSubSession*>(clientData);
			subSession->checkForAuxSDPLine();
		}

		Source::Source()
		{
		}

		OnDemandServerMediaSubsession* Source::getInternalSubSession()
		{
			return internalSubSession;
		}

		void Source::setEnvironment(UsageEnvironment& environment)
		{
			// TODO Memory leak?
			internalSubSession = new InternalSubSession(environment, *this);
		}

		Source::InternalSource::InternalSource(UsageEnvironment& environment, Source& delegate) :
				FramedSource(environment),
				delegate(delegate)
		{
		}

		void Source::InternalSource::doGetNextFrame()
		{
			SourceFrame sourceFrame;
			sourceFrame.frameData = fTo;
			sourceFrame.maxFrameSize = fMaxSize;

			if (delegate.getNextFrame(sourceFrame))
			{
				fDurationInMicroseconds = sourceFrame.durationInMicroseconds;
				fFrameSize = sourceFrame.frameSize;
				fNumTruncatedBytes = sourceFrame.numTruncatedBytes;
				fPresentationTime = sourceFrame.presentationTime;

				afterGetting(this);
			}
		}

		Source::InternalSubSession::InternalSubSession(UsageEnvironment& environment, Source& delegate) :
				OnDemandServerMediaSubsession(environment, True),
				auxSDPLine(),
				delegate(delegate),
				rtpSink(),
				stopAuxSDPLooping(0)
		{
		}

		void Source::InternalSubSession::checkForAuxSDPLine()
		{
			if (rtpSink->auxSDPLine() != nullptr)
			{
				auxSDPLine = rtpSink->auxSDPLine();
				stopAuxSDPLooping = 1;
			}
			else
			{
				// Try again after a 100ms delay
				nextTask() = envir().taskScheduler().scheduleDelayedTask(100000, live555::checkForAuxSDPLine, this);
			}
		}

		FramedSource* Source::InternalSubSession::createNewStreamSource(unsigned int /* clientSessionId */,
				unsigned int& /* estimatedBitrate */)
		{
			// TODO Memory leak?
			return H264VideoStreamFramer::createNew(envir(), new InternalSource(envir(), delegate));
		}

		RTPSink* Source::InternalSubSession::createNewRTPSink(Groupsock* rtpGroupsock,
				unsigned char rtpPayloadTypeIfDynamic, FramedSource* /* inputSource */)
		{
			// TODO Memory leak?
			rtpSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);

			return rtpSink;
		}

		char const* Source::InternalSubSession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
		{
			// Play just enough to get the Aux SDP line
			rtpSink->startPlaying(*inputSource, nullptr, nullptr);
			live555::checkForAuxSDPLine(this);

			// Loop until the Aux SDP line is known
			envir().taskScheduler().doEventLoop(&stopAuxSDPLooping);

			return auxSDPLine.c_str();
		}
	}
}
