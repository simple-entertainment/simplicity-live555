#include <iostream>

#include <BasicUsageEnvironment.hh>

#include <simplicity/logging/Logs.h>

#include "Live555ClientEngine.h"

using namespace std;

namespace simplicity
{
	namespace live555
	{
		void onByeReceived(void* clientData)
		{
			Live555ClientEngine* engine = static_cast<Live555ClientEngine*>(clientData);

			engine->onByeReceived();
		}

		void onDescribeResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			Live555ClientEngine* engine =
					static_cast<Live555ClientEngine*>(static_cast<SimpleRTSPClient*>(rtspClient)->getClientData());

			engine->onDescribeResponse(rtspClient, resultCode, resultString);
		}

		void onPlayingFinished(void* clientData)
		{
			Live555ClientEngine* engine = static_cast<Live555ClientEngine*>(clientData);

			engine->onPlayingFinished();
		}

		void onPlayResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			Live555ClientEngine* engine =
					static_cast<Live555ClientEngine*>(static_cast<SimpleRTSPClient*>(rtspClient)->getClientData());

			engine->onPlayResponse(rtspClient, resultCode, resultString);
		}

		void onSetupResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			Live555ClientEngine* engine =
					static_cast<Live555ClientEngine*>(static_cast<SimpleRTSPClient*>(rtspClient)->getClientData());

			engine->onSetupResponse(rtspClient, resultCode, resultString);
		}

		void stopEventLoop(void* clientData)
		{
			Live555ClientEngine* engine = static_cast<Live555ClientEngine*>(clientData);

			engine->stopEventLoop();
		}

		Live555ClientEngine::Live555ClientEngine(const string& url) :
				// TODO Memory leak?
				environment(BasicUsageEnvironment::createNew(*BasicTaskScheduler::createNew())),
				rtspClient(nullptr),
				session(nullptr),
				sink(nullptr),
				subSession(nullptr),
				url(url)
		{
		}

		void Live555ClientEngine::advance()
		{
			// To prevent the event loop from continuing indefdinately, schedule a taskto stop it.
			// This should effectively mean that only tasks that are already waiting will be processed.
			environment->taskScheduler().scheduleDelayedTask(0, live555::stopEventLoop, this);

			watchVariable = 0;
			environment->taskScheduler().doEventLoop(&watchVariable);
		}

		void Live555ClientEngine::onAddEntity(Entity& entity)
		{
		}

		void Live555ClientEngine::onByeReceived()
		{
			onPlayingFinished();
		}

		void Live555ClientEngine::onDescribeResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			string result(resultString);
			delete[] resultString;

			if (resultCode != 0)
			{
				Logs::error("simplicity::live555", "Failed to get a SDP description: %s", result.c_str());
				onStop();
				return;
			}

			// Create a session.
			// TODO Memory leak?
			session = MediaSession::createNew(*environment, result.c_str());
			if (session == nullptr)
			{
				Logs::error("simplicity::live555",
						"Failed to create a MediaSession object from the SDP description: %s",
						environment->getResultMsg());
				onStop();
				return;
			}

			// Create the sub-sessions.
			// TODO Handle multiple sub-sessions
			unique_ptr<MediaSubsessionIterator> iterator(new MediaSubsessionIterator(*session));
			subSession = iterator->next();

			if (!subSession->initiate())
			{
				Logs::error("simplicity::live555", "Failed to initiate the \"%s\" sub-session: %s",
						subSession->mediumName(), environment->getResultMsg());
				onStop();
				return;
			}

			rtspClient->sendSetupCommand(*subSession, live555::onSetupResponse);
		}

		void Live555ClientEngine::onPlay()
		{
			// TODO Memory leak?
			rtspClient = new SimpleRTSPClient(*environment, url, 1, "simplicity::live555");
			rtspClient->setClientData(this);

			/*
			 * The flow of commands to start the stream is:
			 *
			 * DESCRIBE
			 * SETUP (for each stream)
			 * PLAY
			 */
			rtspClient->sendDescribeCommand(live555::onDescribeResponse);
		}

		void Live555ClientEngine::onPlayingFinished()
		{
			// Begin by closing this sub-session's stream:
			Medium::close(subSession->sink);
			subSession->sink = nullptr;

			// Next, check whether *all* sub-sessions' streams have now been closed:
			for (MediaSubsessionIterator iterator(subSession->parentSession()); subSession != nullptr;
					subSession = iterator.next())
			{
				if (subSession->sink != nullptr)
				{
					return; // this sub-session is still active
				}
			}

			// All sub-sessions' streams have now been closed, so shutdown the client:
			onStop();
		}

		void Live555ClientEngine::onPlayResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			string result;
			if (resultString != nullptr)
			{
				result = resultString;
				delete[] resultString;
			}

			if (resultCode != 0)
			{
				Logs::error("simplicity::live555", "Failed to start playing session: %s", result.c_str());
				onStop();
			}
		}

		void Live555ClientEngine::onRemoveEntity(Entity& entity)
		{
		}

		// TODO Handle multiple sub-sessions
		void Live555ClientEngine::onSetupResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
		{
			string result;
			if (resultString != nullptr)
			{
				result = resultString;
				delete[] resultString;
			}

			if (resultCode != 0)
			{
				Logs::error("simplicity::live555", "Failed to set up the \"%s\" subsession: %s", subSession->mediumName(),
						result.c_str());
			  return;
			}

			subSession->sink = sink->getInternalSink();
			sink->setSubSession(subSession);
			FramedSource* source = subSession->readSource();
			subSession->sink->startPlaying(*subSession->readSource(), live555::onPlayingFinished, this);

			if (subSession->rtcpInstance() != nullptr)
			{
				subSession->rtcpInstance()->setByeHandler(live555::onByeReceived, this);
			}

			if (session->absStartTime() != nullptr)
			{
				// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
				rtspClient->sendPlayCommand(*session, live555::onPlayResponse, session->absStartTime(),
						session->absEndTime());
			}
			else
			{
				rtspClient->sendPlayCommand(*session, live555::onPlayResponse);
			}
		}

		void Live555ClientEngine::onStop()
		{
			if (session != nullptr)
			{
				MediaSubsession* subSession = nullptr;
				bool someSubsessionsWereActive = false;
				for (MediaSubsessionIterator iterator(*session); subSession != nullptr; subSession = iterator.next())
				{
					if (subSession->sink != nullptr)
					{
						Medium::close(subSession->sink);
						subSession->sink = nullptr;

						if (subSession->rtcpInstance() != nullptr)
						{
							// in case the server sends a RTCP "BYE" while handling "TEARDOWN"
							subSession->rtcpInstance()->setByeHandler(nullptr, nullptr);
						}

						someSubsessionsWereActive = true;
					}
				}

				if (someSubsessionsWereActive)
				{
					rtspClient->sendTeardownCommand(*session, nullptr);
				}
			}

			Medium::close(rtspClient);
			Medium::close(session);
		}

		void Live555ClientEngine::setSink(unique_ptr<Sink> sink)
		{
			this->sink = move(sink);
			this->sink->setEnvironment(*environment);
		}

		void Live555ClientEngine::stopEventLoop()
		{
			watchVariable = 1;
		}
	}
}
