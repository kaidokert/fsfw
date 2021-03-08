#ifndef FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_
#define FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_

#include "../../objectmanager/SystemObject.h"
#include "../../tasks/ExecutableObjectIF.h"

#include <string>
#include <vector>

//! Debugging preprocessor define.
#define FSFW_TCP_SERVER_WIRETAPPING_ENABLED    0

class TcWinTcpServer:
        public SystemObject,
        public ExecutableObjectIF {
public:
    /* The ports chosen here should not be used by any other process. */
    static const std::string DEFAULT_TCP_SERVER_PORT;
    static const std::string DEFAULT_TCP_CLIENT_PORT;

    TcWinTcpServer(object_id_t objectId, object_id_t tmtcUnixUdpBridge,
            std::string customTcpServerPort = "");
    virtual~ TcWinTcpServer();

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;

private:

    std::string tcpPort;
    SOCKET listenerTcpSocket = 0;
    struct sockaddr_in tcpAddress;
    int tcpAddrLen = sizeof(tcpAddress);
    int currentBacklog = 3;

    std::vector<uint8_t> receptionBuffer;
    int tcpSockOpt = 0;

    enum class ErrorSources {
        GETADDRINFO_CALL,
        SOCKET_CALL,
        SETSOCKOPT_CALL,
        BIND_CALL,
        LISTEN_CALL,
        ACCEPT_CALL
    };

    void handleError(ErrorSources errorSrc);
};

#endif /* FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_ */
