#ifndef FSFW_OSAL_COMMON_TMTCUDPBRIDGE_H_
#define FSFW_OSAL_COMMON_TMTCUDPBRIDGE_H_

#include "TcpIpBase.h"
#include "fsfw/platform.h"
#include "fsfw/tmtcservices/TmTcBridge.h"

#ifdef PLATFORM_WIN
#include <ws2tcpip.h>
#elif defined(PLATFORM_UNIX)
#include <sys/socket.h>
#endif

#include <string>

/**
 * @brief   This class can be used with the UdpTcPollingTask to implement a UDP server
 *          for receiving and sending PUS TMTC.
 * @details
 * This bridge task will take care of sending telemetry back to a UDP client if a connection
 * was established and store them in a FIFO if this was not done yet. It is also be the default
 * destination for telecommands, but the telecommands will be relayed to a specified tcDestination
 * directly.
 */
class UdpTmTcBridge : public TmTcBridge, public TcpIpBase {
  friend class UdpTcPollingTask;

 public:
  /* The ports chosen here should not be used by any other process. */
  static const std::string DEFAULT_SERVER_PORT;

  UdpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
                const std::string& udpServerPort = "", object_id_t tmStoreId = objects::TM_STORE,
                object_id_t tcStoreId = objects::TC_STORE);
  ~UdpTmTcBridge() override;

  /**
   * Set properties of internal mutex.
   */
  void setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs);

  ReturnValue_t initialize() override;

  void checkAndSetClientAddress(sockaddr& clientAddress);

  std::string getUdpPort() const;

 protected:
  ReturnValue_t sendTm(const uint8_t* data, size_t dataLen) override;

 private:
  std::string udpServerPort;

  struct sockaddr clientAddress = {};
  socklen_t clientAddressLen = 0;

  //! Access to the client address is mutex protected as it is set by another task.
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  dur_millis_t mutexTimeoutMs = 20;
  MutexIF* mutex;
};

#endif /* FSFW_OSAL_COMMON_TMTCUDPBRIDGE_H_ */
