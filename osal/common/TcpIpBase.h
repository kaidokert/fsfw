#ifndef FSFW_OSAL_COMMON_TCPIPIF_H_
#define FSFW_OSAL_COMMON_TCPIPIF_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#ifdef _WIN32

#include <winsock2.h>

#elif defined(__unix__)

#include <sys/socket.h>

#endif

class TcpIpBase {
protected:

#ifdef _WIN32
    static constexpr int SHUT_RECV = SD_RECEIVE;
    static constexpr int SHUT_SEND = SD_SEND;
    static constexpr int SHUT_BOTH = SD_BOTH;

    using socket_t = SOCKET;
#elif defined(__unix__)
    using socket_t = int;

    static constexpr int INVALID_SOCKET = -1;
    static constexpr int SOCKET_ERROR = -1;

    static constexpr int SHUT_RECV = SHUT_RD;
    static constexpr int SHUT_SEND = SHUT_WR;
    static constexpr int SHUT_BOTH = SHUT_RDWR;
#endif

    TcpIpBase();
    virtual ~TcpIpBase();

    ReturnValue_t initialize();

    int closeSocket(socket_t socket);

    int getLastSocketError();

    socket_t serverSocket = 0;

};


#endif /* FSFW_OSAL_COMMON_TCPIPIF_H_ */
