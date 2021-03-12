#ifndef FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_
#define FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_

#include "../../tmtcservices/TmTcBridge.h"

#include <winsock2.h>

//! Debugging preprocessor define.
#define FSFW_UDP_SEND_WIRETAPPING_ENABLED    0

class TmTcWinUdpBridge: public TmTcBridge {
    friend class TcWinUdpPollingTask;
public:
    /* The ports chosen here should not be used by any other process. */
    static const std::string DEFAULT_UDP_SERVER_PORT;
    static const std::string DEFAULT_UDP_CLIENT_PORT;

    TmTcWinUdpBridge(object_id_t objectId, object_id_t tcDestination,
            object_id_t tmStoreId, object_id_t tcStoreId,
            std::string udpServerPort = "", std::string udpClientPort = "");
    virtual~ TmTcWinUdpBridge();

    ReturnValue_t initialize() override;

    void checkAndSetClientAddress(sockaddr_in& clientAddress);

protected:
    virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:
    SOCKET serverSocket = 0;
    std::string udpServerPort;
    std::string udpClientPort;

    const int serverSocketOptions = 0;

    struct sockaddr_in clientAddress;
    int clientAddressLen = 0;

    struct sockaddr_in serverAddress;
    int serverAddressLen = 0;

    //! Access to the client address is mutex protected as it is set by another task.
    MutexIF* mutex;
};

#endif /* FSFW_OSAL_HOST_TMTCWINUDPBRIDGE_H_ */

