#ifndef FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_
#define FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_

#include "../../tmtcservices/AcceptsTelecommandsIF.h"
#include "../../tmtcservices/TmTcBridge.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

class TmTcUnixUdpBridge:
        public TmTcBridge {
    friend class TcUnixUdpPollingTask;
public:

    /* The ports chosen here should not be used by any other process.
	List of used ports on Linux: /etc/services */
    static const std::string DEFAULT_UDP_SERVER_PORT;

    TmTcUnixUdpBridge(object_id_t objectId, object_id_t tcDestination,
            object_id_t tmStoreId, object_id_t tcStoreId,
            std::string serverPort = "");
    virtual~ TmTcUnixUdpBridge();

    /**
     * Set properties of internal mutex.
     */
    void setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs);

    ReturnValue_t initialize() override;

    void checkAndSetClientAddress(sockaddr_in& clientAddress);

    void setClientAddressToAny(bool ipAddrAnySet);

protected:
    virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:
    int serverSocket = 0;
    std::string udpServerPort;

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = 0;

    bool ipAddrAnySet = false;

    //! Access to the client address is mutex protected as it is set by another task.
    MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
    dur_millis_t mutexTimeoutMs = 20;
    MutexIF* mutex;
};

#endif /* FRAMEWORK_OSAL_LINUX_TMTCUNIXUDPBRIDGE_H_ */
