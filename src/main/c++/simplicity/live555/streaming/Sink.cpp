#include "Sink.h"

using namespace std;

const int FRAME_SIZE = 1000000;

namespace simplicity
{
	namespace live555
	{
		void onFrameReceived(void* clientData, unsigned int rtpFrameSize, unsigned int numTruncatedBytes,
				struct timeval presentationTime, unsigned int durationInMicroseconds)
		{
			Sink::InternalSink* sink = static_cast<Sink::InternalSink*>(clientData);

			sink->onFrameReceived(rtpFrameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
		}

		Sink::Sink() :
			internalSink(nullptr),
			subSession(nullptr)
		{
		}

		Sink::~Sink()
		{
		}

		MediaSink* Sink::getInternalSink()
		{
			return internalSink;
		}

		MediaSubsession* Sink::getSubSession()
		{
			return subSession;
		}

		void Sink::setEnvironment(UsageEnvironment& environment)
		{
			// TODO Memory leak?
			internalSink = new InternalSink(environment, *this);
		}

		void Sink::setSubSession(MediaSubsession* subSession)
		{
			this->subSession= subSession;
		}

		Sink::InternalSink::InternalSink(UsageEnvironment& environment, Sink& delegate) :
			MediaSink(environment),
			delegate(delegate),
			rtpFrame(FRAME_SIZE)
		{
		}

		void Sink::InternalSink::onFrameReceived(unsigned int rtpFrameSize, unsigned int numTruncatedBytes,
				struct timeval presentationTime, unsigned int durationInMicroseconds)
		{
			delegate.onFrameReceived(rtpFrame.data(), rtpFrameSize, numTruncatedBytes, presentationTime,
					durationInMicroseconds);

			continuePlaying();
		}

		Boolean Sink::InternalSink::continuePlaying()
		{
			fSource->getNextFrame(rtpFrame.data(), FRAME_SIZE, live555::onFrameReceived, this, nullptr, nullptr);

			return True;
		}
	}
}
