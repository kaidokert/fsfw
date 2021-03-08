#ifndef FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_
#define FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_

#include "../../objectmanager/SystemObject.h"
#include "../../tasks/ExecutableObjectIF.h"

#include <vector>

class TcWinTcpServer:
        public SystemObject,
        public ExecutableObjectIF {
public:
    /* The ports chosen here should not be used by any other process. */
    static constexpr uint16_t DEFAULT_TCP_SERVER_PORT = 7301;
    static constexpr uint16_t DEFAULT_TCP_CLIENT_PORT = 7302;

    TcWinTcpServer(object_id_t objectId, object_id_t tmtcUnixUdpBridge,
            uint16_t customTcpServerPort = 0xffff);
    virtual~ TcWinTcpServer();

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;

private:

    SOCKET serverTcpSocket = 0;
    struct sockaddr_in tcpAddress;
    int tcpAddrLen = sizeof(tcpAddress);
    int backlog = 3;

    std::vector<uint8_t> receptionBuffer;
    int tcpSockOpt = 0;

    enum class ErrorSources {
        SOCKET_CALL,
        SETSOCKOPT_CALL,
        BIND_CALL,
        LISTEN_CALL,
        ACCEPT_CALL
    };

    void handleError(ErrorSources errorSrc);
};

#endif /* FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_ */
