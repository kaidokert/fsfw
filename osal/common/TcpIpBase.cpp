#include "TcpIpBase.h"

#ifdef __unix__

#include <errno.h>
#include <unistd.h>

#endif

TcpIpBase::TcpIpBase() {

}

TcpIpBase::~TcpIpBase() {
    closeSocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

ReturnValue_t TcpIpBase::initialize() {
#ifdef _WIN32
    /* Initiates Winsock DLL. */
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable Winsock DLL. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: WSAStartup failed with error: " <<
                err << std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
#endif
    return HasReturnvaluesIF::RETURN_OK;
}

int TcpIpBase::closeSocket(socket_t socket) {
#ifdef _WIN32
    return closesocket(socket);
#elif defined(__unix__)
    return close(socket);
#endif
}

int TcpIpBase::getLastSocketError() {
#ifdef _WIN32
    return WSAGetLastError();
#elif defined(__unix__)
    return errno;
#endif
}

