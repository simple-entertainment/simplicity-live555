#ifndef LIVE555SERVERENGINE_H_
#define LIVE555SERVERENGINE_H_

#include <simplicity/engine/Engine.h>

#include "Source.h"

namespace simplicity
{
	namespace live555
	{
		class Live555ServerEngine : public Engine
		{
			public:
				Live555ServerEngine();

				void advance() override;

				void onAddEntity(Entity& entity) override;

				void onPlay() override;

				void onRemoveEntity(Entity& entity) override;

				void onStop() override;

				void setSource(std::unique_ptr<Source> source);

				void stopEventLoop();

			private:
				UsageEnvironment* environment;

				std::unique_ptr<Source> source;

				char watchVariable;
		};
	}
}

#endif /* LIVE555SERVERENGINE_H_ */
