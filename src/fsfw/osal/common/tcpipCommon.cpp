#include "fsfw/osal/common/tcpipCommon.h"

#include <cerrno>

#include "fsfw/platform.h"
#include "fsfw/serviceinterface.h"

#ifdef PLATFORM_WIN
#include <ws2tcpip.h>
#endif

void tcpip::determineErrorStrings(Protocol protocol, ErrorSources errorSrc, std::string &protStr,
                                  std::string &srcString) {
  if (protocol == Protocol::TCP) {
    protStr = "TCP";
  } else if (protocol == Protocol::UDP) {
    protStr = "UDP";
  } else {
    protStr = "Unknown protocol";
  }

  if (errorSrc == ErrorSources::SETSOCKOPT_CALL) {
    srcString = "setsockopt call";
  }
  if (errorSrc == ErrorSources::BIND_CALL) {
    srcString = "bind call";
  } else if (errorSrc == ErrorSources::SOCKET_CALL) {
    srcString = "socket call";
  } else if (errorSrc == ErrorSources::LISTEN_CALL) {
    srcString = "listen call";
  } else if (errorSrc == ErrorSources::ACCEPT_CALL) {
    srcString = "accept call";
  } else if (errorSrc == ErrorSources::RECVFROM_CALL) {
    srcString = "recvfrom call";
  } else if (errorSrc == ErrorSources::SEND_CALL) {
    srcString = "send call";
  } else if (errorSrc == ErrorSources::SENDTO_CALL) {
    srcString = "sendto call";
  } else if (errorSrc == ErrorSources::GETADDRINFO_CALL) {
    srcString = "getaddrinfo call";
  } else if (errorSrc == ErrorSources::SHUTDOWN_CALL) {
    srcString = "shutdown call";
  } else {
    srcString = "unknown call";
  }
}

void tcpip::printAddress(struct sockaddr *addr) {
  char ipAddress[INET6_ADDRSTRLEN] = {};
  const char *stringPtr = NULL;
  switch (addr->sa_family) {
    case AF_INET: {
      auto *addrIn = reinterpret_cast<struct sockaddr_in *>(addr);
      stringPtr = inet_ntop(AF_INET, &(addrIn->sin_addr), ipAddress, INET_ADDRSTRLEN);
      break;
    }
    case AF_INET6: {
      auto *addrIn = reinterpret_cast<struct sockaddr_in6 *>(addr);
      stringPtr = inet_ntop(AF_INET6, &(addrIn->sin6_addr), ipAddress, INET6_ADDRSTRLEN);
      break;
    }
  }
  if (stringPtr == nullptr) {
    FSFW_LOGDT("Could not convert IP address to text representation, error code {} | {}", errno,
               strerror(errno));
  } else {
    FSFW_LOGDT("IP Address Sender {}\n", ipAddress);
  }
}
