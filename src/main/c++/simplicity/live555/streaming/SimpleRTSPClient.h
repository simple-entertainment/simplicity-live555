#ifndef SIMPLERTSPCLIENT_H_
#define SIMPLERTSPCLIENT_H_

#include <string>

#include <liveMedia.hh>

#include <simplicity/common/Defines.h>

namespace simplicity
{
	namespace live555
	{
		class SIMPLE_API SimpleRTSPClient : public RTSPClient
		{
			public:
				SimpleRTSPClient(UsageEnvironment& environment, const std::string& rtspURL, int verbosityLevel = 0,
						const std::string& applicationName = nullptr, portNumBits tunnelOverHTTPPortNum = 0);

				void* getClientData();

				void setClientData(void* clientData);

			private:
				void* clientData;
		};
	}
}

#endif /* SIMPLERTSPCLIENT_H_ */
