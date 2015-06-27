/*
 * SimpleRTSPClient.cpp
 *
 *  Created on: 18/04/2015
 *      Author: simplegsb
 */

#include <simplicity/live555/streaming/SimpleRTSPClient.h>

using namespace std;

namespace simplicity
{
	namespace live555
	{
		SimpleRTSPClient::SimpleRTSPClient(UsageEnvironment& environment, const string& rtspURL, int verbosityLevel,
				const string& applicationName, portNumBits tunnelOverHTTPPortNum) :
				RTSPClient(environment, rtspURL.c_str(), verbosityLevel, applicationName.c_str(),
						tunnelOverHTTPPortNum, -1),
				clientData(nullptr)
		{
		}

		void* SimpleRTSPClient::getClientData()
		{
			return clientData;
		}

		void SimpleRTSPClient::setClientData(void* clientData)
		{
			this->clientData = clientData;
		}
	}
}
