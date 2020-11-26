#ifndef FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_
#define FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_

#include "../../tmtcservices/AcceptsTelecommandsIF.h"
#include "../../tmtcservices/TmTcBridge.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

class TmTcUnixUdpBridge: public TmTcBridge {
	friend class TcUnixUdpPollingTask;
public:
	// The ports chosen here should not be used by any other process.
	// List of used ports on Linux: /etc/services
	static constexpr uint16_t DEFAULT_UDP_SERVER_PORT = 7301;
	static constexpr uint16_t DEFAULT_UDP_CLIENT_PORT = 7302;

	TmTcUnixUdpBridge(object_id_t objectId, object_id_t tcDestination,
			object_id_t tmStoreId, object_id_t tcStoreId,
			uint16_t serverPort = 0xFFFF,uint16_t clientPort = 0xFFFF);
	virtual~ TmTcUnixUdpBridge();

	void checkAndSetClientAddress(sockaddr_in& clientAddress);

	void setClientAddressToAny(bool ipAddrAnySet);
protected:
	virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:
	int serverSocket = 0;

	const int serverSocketOptions = 0;

	struct sockaddr_in clientAddress;
	socklen_t clientAddressLen = 0;

	struct sockaddr_in serverAddress;
	socklen_t serverAddressLen = 0;

	bool ipAddrAnySet = false;

	//! Access to the client address is mutex protected as it is set
	//! by another task.
	MutexIF* mutex;

	void handleSocketError();
	void handleBindError();
	void handleSendError();
};

#endif /* FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_ */
