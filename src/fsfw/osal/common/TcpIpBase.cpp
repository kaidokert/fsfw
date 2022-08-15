#include "fsfw/osal/common/TcpIpBase.h"

#include "fsfw/platform.h"

#ifdef PLATFORM_UNIX
#include <errno.h>
#include <unistd.h>
#endif

TcpIpBase::TcpIpBase() {}

ReturnValue_t TcpIpBase::initialize() {
#ifdef _WIN32
  /* Initiates Winsock DLL. */
  WSAData wsaData;
  WORD wVersionRequested = MAKEWORD(2, 2);
  int err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    /* Tell the user that we could not find a usable Winsock DLL. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: WSAStartup failed with error: " << err
               << std::endl;
#endif
    return returnvalue::FAILED;
  }
#endif
  return returnvalue::OK;
}

TcpIpBase::~TcpIpBase() {
  closeSocket(serverSocket);
#ifdef _WIN32
  WSACleanup();
#endif
}

int TcpIpBase::closeSocket(socket_t socket) {
#ifdef PLATFORM_WIN
  return closesocket(socket);
#elif defined(PLATFORM_UNIX)
  return close(socket);
#endif
}

int TcpIpBase::getLastSocketError() {
#ifdef PLATFORM_WIN
  return WSAGetLastError();
#elif defined(PLATFORM_UNIX)
  return errno;
#endif
}
