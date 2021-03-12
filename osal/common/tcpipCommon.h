#ifndef FSFW_OSAL_COMMON_TCPIPCOMMON_H_
#define FSFW_OSAL_COMMON_TCPIPCOMMON_H_

#include "../../timemanager/clockDefinitions.h"
#include <string>

namespace tcpip {

const char* const DEFAULT_UDP_SERVER_PORT = "7301";
const char* const DEFAULT_TCP_SERVER_PORT = "7303";

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
    SENDTO_CALL
};

void determineErrorStrings(Protocol protocol, ErrorSources errorSrc, std::string& protStr,
        std::string& srcString);

}



#endif /* FSFW_OSAL_COMMON_TCPIPCOMMON_H_ */
