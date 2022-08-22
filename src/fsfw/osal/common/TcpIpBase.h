#ifndef FSFW_OSAL_COMMON_TCPIPIF_H_
#define FSFW_OSAL_COMMON_TCPIPIF_H_

#include "../../platform.h"
#include "../../returnvalues/returnvalue.h"

#ifdef PLATFORM_WIN
#include <winsock2.h>
#elif defined(PLATFORM_UNIX)
#include <sys/socket.h>
#endif

class TcpIpBase {
 protected:
#ifdef PLATFORM_WIN
  static constexpr int SHUT_RECV = SD_RECEIVE;
  static constexpr int SHUT_SEND = SD_SEND;
  static constexpr int SHUT_BOTH = SD_BOTH;

  using socket_t = SOCKET;
#elif defined(PLATFORM_UNIX)
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
