#include <BasicUsageEnvironment.hh>

#include <simplicity/logging/Logs.h>

#include "Live555ServerEngine.h"

using namespace std;

namespace simplicity
{
	namespace live555
	{
		void stopEventLoop(void* clientData)
		{
			Live555ServerEngine* engine = static_cast<Live555ServerEngine*>(clientData);

			engine->stopEventLoop();
		}

		Live555ServerEngine::Live555ServerEngine() :
				// TODO Memory leak?
				environment(BasicUsageEnvironment::createNew(*BasicTaskScheduler::createNew())),
				source()
		{

		}

		void Live555ServerEngine::advance()
		{
			// To prevent the event loop from continuing indefdinately, schedule a taskto stop it.
			// This should effectively mean that only tasks that are already waiting will be processed.
			environment->taskScheduler().scheduleDelayedTask(0, live555::stopEventLoop, this);

			watchVariable = 0;
			environment->taskScheduler().doEventLoop(&watchVariable);
		}

		void Live555ServerEngine::onAddEntity(Entity& entity)
		{
		}

		void Live555ServerEngine::onPlay()
		{
			UserAuthenticationDatabase* authenticationDatabase = nullptr;
/*#ifdef ACCESS_CONTROL
			// To implement client access control to the RTSP server, do the following:
			authenticationDatabase = new UserAuthenticationDatabase;
			authenticationDatabase->addUserRecord("username1", "password1"); // replace these with real strings
			// Repeat the above with each <username>, <password> that you wish to allow
			// access to the server.
#endif*/

			// TODO Memory leak?
			RTSPServer* rtspServer = RTSPServer::createNew(*environment, 8554, authenticationDatabase);
			if (rtspServer == NULL)
			{
				Logs::error("simplicity::live555", "Failed to create an RTSP server: %s", environment->getResultMsg());
				return;
			}

			// TODO Memory leak?
			ServerMediaSession* session = ServerMediaSession::createNew(*environment, "simplicity::live555",
					"simplicity::live555", "simplicity::live555");
			session->addSubsession(source->getInternalSubSession());
			rtspServer->addServerMediaSession(session);

			Logs::info("simplicity::live555", "simplicity::live555 stream available at %s", rtspServer->rtspURL(session));
		}

		void Live555ServerEngine::onRemoveEntity(Entity& entity)
		{
		}

		void Live555ServerEngine::onStop()
		{
		}

		void Live555ServerEngine::setSource(unique_ptr<Source> source)
		{
			this->source = move(source);
			this->source->setEnvironment(*environment);
		}

		void Live555ServerEngine::stopEventLoop()
		{
			watchVariable = 1;
		}
	}
}
