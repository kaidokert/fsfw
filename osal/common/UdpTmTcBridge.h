#ifndef FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_
#define FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_

#include "TcpIpBase.h"
#include "../../tmtcservices/TmTcBridge.h"

#ifdef _WIN32

#include <ws2tcpip.h>

#elif defined(__unix__)

#include <sys/socket.h>

#endif

#include <string>

/**
 * @brief   This class should be used with the UdpTcPollingTask to implement a UDP server
 *          for receiving and sending PUS TMTC.
 */
class UdpTmTcBridge:
        public TmTcBridge,
        public TcpIpBase {
    friend class UdpTcPollingTask;
public:
    /* The ports chosen here should not be used by any other process. */
    static const std::string DEFAULT_UDP_SERVER_PORT;

    UdpTmTcBridge(object_id_t objectId, object_id_t tcDestination, std::string udpServerPort = "",
            object_id_t tmStoreId = objects::TM_STORE, object_id_t tcStoreId = objects::TC_STORE);
    virtual~ UdpTmTcBridge();

    /**
     * Set properties of internal mutex.
     */
    void setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs);

    ReturnValue_t initialize() override;

    void checkAndSetClientAddress(sockaddr& clientAddress);

protected:
    virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:
    std::string udpServerPort;

    struct sockaddr clientAddress;
    socklen_t clientAddressLen = 0;

    //! Access to the client address is mutex protected as it is set by another task.
    MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
    dur_millis_t mutexTimeoutMs = 20;
    MutexIF* mutex;
};

#endif /* FSFW_OSAL_HOST_TMTCWINUDPBRIDGE_H_ */

