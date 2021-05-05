#ifndef FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_
#define FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_

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
class TcpTmTcBridge:
        public TmTcBridge {
    //friend class UdpTcPollingTask;
public:
    /* The ports chosen here should not be used by any other process. */
    static const std::string DEFAULT_UDP_SERVER_PORT;

    TcpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
            object_id_t tmStoreId = objects::TM_STORE,
            object_id_t tcStoreId = objects::TC_STORE);
    virtual~ TcpTmTcBridge();

    /**
     * Set properties of internal mutex.
     */
    void setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs);

    ReturnValue_t initialize() override;

protected:
    virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:

    //! Access to the client address is mutex protected as it is set by another task.
    MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
    dur_millis_t mutexTimeoutMs = 20;
    MutexIF* mutex;
};

#endif /* FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_ */

