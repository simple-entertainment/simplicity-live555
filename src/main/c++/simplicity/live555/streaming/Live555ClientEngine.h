#ifndef LIVE555CLIENTENGINE_H_
#define LIVE555CLIENTENGINE_H_

#include <simplicity/engine/Engine.h>

#include "SimpleRTSPClient.h"
#include "Sink.h"

namespace simplicity
{
	namespace live555
	{
		class SIMPLE_API Live555ClientEngine : public Engine
		{
			public:
				Live555ClientEngine(const std::string& url);

				void advance() override;

				void onAddEntity(Entity& entity) override;

				void onByeReceived();

				void onDescribeResponse(RTSPClient* rtspClient, int resultCode, char* resultString);

				void onPlay() override;

				void onPlayingFinished();

				void onPlayResponse(RTSPClient* rtspClient, int resultCode, char* resultString);

				void onRemoveEntity(Entity& entity) override;

				void onSetupResponse(RTSPClient* rtspClient, int resultCode, char* resultString);

				void onStop() override;

				void setSink(std::unique_ptr<Sink> sink);

				void stopEventLoop();

			private:
				UsageEnvironment* environment;

				SimpleRTSPClient* rtspClient;

				MediaSession* session;

				std::unique_ptr<Sink> sink;

				MediaSubsession* subSession;

				std::string url;

				char watchVariable;
		};
	}
}

#endif /* LIVE555CLIENTENGINE_H_ */
