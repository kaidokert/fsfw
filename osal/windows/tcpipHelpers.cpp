#include "tcpipHelpers.h"
#include <FSFWConfig.h>

#include "../../serviceinterface/ServiceInterface.h"

#include <winsock2.h>
#include <string>

void tcpip::handleError(Protocol protocol, ErrorSources errorSrc, dur_millis_t sleepDuration) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    int errCode = WSAGetLastError();
    std::string protocolString;
    if(protocol == Protocol::TCP) {
        protocolString = "TCP";
    }
    else if(protocol == Protocol::UDP) {
        protocolString = "UDP";
    }
    else {
        protocolString = "Unknown protocol";
    }

    std::string errorSrcString;
    if(errorSrc == ErrorSources::SETSOCKOPT_CALL) {
        errorSrcString = "setsockopt call";
    }
    else if(errorSrc == ErrorSources::SOCKET_CALL) {
        errorSrcString = "socket call";
    }
    else if(errorSrc == ErrorSources::LISTEN_CALL) {
        errorSrcString = "listen call";
    }
    else if(errorSrc == ErrorSources::ACCEPT_CALL) {
        errorSrcString = "accept call";
    }
    else if(errorSrc == ErrorSources::RECVFROM_CALL) {
        errorSrcString = "recvfrom call";
    }
    else if(errorSrc == ErrorSources::GETADDRINFO_CALL) {
        errorSrcString = "getaddrinfo call";
    }
    else {
        errorSrcString = "unknown call";
    }

    std::string infoString;
    switch(errCode) {
    case(WSANOTINITIALISED): {
        infoString = "WSANOTINITIALISED";
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                " | " << infoString << std::endl;
        break;
    }
    case(WSAEADDRINUSE): {
        infoString = "WSAEADDRINUSE";
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                " | " << infoString << std::endl;
        break;
    }
    case(WSAEFAULT): {
        infoString = "WSAEFAULT";
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                " | " << infoString << std::endl;
        break;
    }
    case(WSAEADDRNOTAVAIL): {
        infoString = "WSAEADDRNOTAVAIL";
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                " | " << infoString << std::endl;
        break;
    }
    case(WSAEINVAL): {
        infoString = "WSAEINVAL";
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                " | " << infoString << std::endl;
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
         */
        sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString <<
                "Error code" << errCode << std::endl;
        break;
    }
    }
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
    if(sleepDuration > 0) {
        Sleep(sleepDuration);
    }
}

