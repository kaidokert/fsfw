#ifndef FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_
#define FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_

#include "../../timemanager/clockDefinitions.h"

namespace tcpip {

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

void handleError(Protocol protocol, ErrorSources errorSrc, dur_millis_t sleepDuration = 0);

}



#endif /* FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_ */
