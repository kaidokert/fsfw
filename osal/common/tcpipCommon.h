#ifndef FSFW_OSAL_COMMON_TCPIPCOMMON_H_
#define FSFW_OSAL_COMMON_TCPIPCOMMON_H_

#include "../../timemanager/clockDefinitions.h"
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

namespace tcpip {

const char* const DEFAULT_SERVER_PORT = "7301";

enum class Protocol {
    UDP,
    TCP
};

enum class ErrorSources {
    GETADDRINFO_CALL,
    SOCKET_CALL,
    SETSOCKOPT_CALL,
    BIND_CALL,
    RECV_CALL,
    RECVFROM_CALL,
    LISTEN_CALL,
    ACCEPT_CALL,
    SENDTO_CALL,
    SHUTDOWN_CALL
};

void determineErrorStrings(Protocol protocol, ErrorSources errorSrc, std::string& protStr,
        std::string& srcString);

void printAddress(struct sockaddr* addr);

}

#endif /* FSFW_OSAL_COMMON_TCPIPCOMMON_H_ */
