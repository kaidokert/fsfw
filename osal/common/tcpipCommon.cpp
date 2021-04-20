#include "tcpipCommon.h"

void tcpip::determineErrorStrings(Protocol protocol, ErrorSources errorSrc, std::string &protStr,
        std::string &srcString) {
    if(protocol == Protocol::TCP) {
        protStr = "TCP";
    }
    else if(protocol == Protocol::UDP) {
        protStr = "UDP";
    }
    else {
        protStr = "Unknown protocol";
    }

    if(errorSrc == ErrorSources::SETSOCKOPT_CALL) {
        srcString = "setsockopt call";
    }
    else if(errorSrc == ErrorSources::SOCKET_CALL) {
        srcString = "socket call";
    }
    else if(errorSrc == ErrorSources::LISTEN_CALL) {
        srcString = "listen call";
    }
    else if(errorSrc == ErrorSources::ACCEPT_CALL) {
        srcString = "accept call";
    }
    else if(errorSrc == ErrorSources::RECVFROM_CALL) {
        srcString = "recvfrom call";
    }
    else if(errorSrc == ErrorSources::GETADDRINFO_CALL) {
        srcString = "getaddrinfo call";
    }
    else {
        srcString = "unknown call";
    }
}
