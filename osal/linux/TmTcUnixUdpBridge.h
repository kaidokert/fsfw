#ifndef FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_
#define FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_

#include <framework/tmtcservices/TmTcBridge.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

class TmTcUnixUdpBridge: public TmTcBridge {
public:
	static constexpr int UDP_SERVER_PORT = 7;
	static constexpr int UDP_CLIENT_PORT = 2008;

	TmTcUnixUdpBridge(object_id_t objectId, object_id_t ccsdsPacketDistributor);
	virtual~ TmTcUnixUdpBridge();

protected:

private:
	int serverSocket = 0;
	const int serverSocketOptions = 0;
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;
};



#endif /* FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_ */
