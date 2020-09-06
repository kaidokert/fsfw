#ifndef FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_
#define FSFW_OSAL_WINDOWS_TMTCWINUDPBRIDGE_H_

#include "../../tmtcservices/TmTcBridge.h"

#include <winsock2.h>
#include <windows.h>

class TmTcWinUdpBridge: public TmTcBridge {
    friend class TcWinUdpPollingTask;
public:
    // The ports chosen here should not be used by any other process.
    static constexpr uint16_t DEFAULT_UDP_SERVER_PORT = 7301;
    static constexpr uint16_t DEFAULT_UDP_CLIENT_PORT = 7302;

    TmTcWinUdpBridge(object_id_t objectId, object_id_t tcDestination,
            object_id_t tmStoreId, object_id_t tcStoreId,
            uint16_t serverPort = 0xFFFF,uint16_t clientPort = 0xFFFF);
    virtual~ TmTcWinUdpBridge();

    void checkAndSetClientAddress(sockaddr_in clientAddress);

protected:
    virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;

private:
    SOCKET serverSocket = 0;

    const int serverSocketOptions = 0;

    struct sockaddr_in clientAddress;
    int clientAddressLen = 0;

    struct sockaddr_in serverAddress;
    int serverAddressLen = 0;

    //! Access to the client address is mutex protected as it is set
    //! by another task.
    MutexIF* mutex;

    void handleSocketError();
    void handleBindError();
    void handleSendError();
};



#endif /* FSFW_OSAL_HOST_TMTCWINUDPBRIDGE_H_ */

