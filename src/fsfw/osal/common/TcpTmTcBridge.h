#ifndef FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_
#define FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_

#include "TcpIpBase.h"
#include "fsfw/tmtcservices/TmTcBridge.h"

#ifdef _WIN32

#include <ws2tcpip.h>

#elif defined(__unix__)

#include <sys/socket.h>

#endif

#include <string>

/**
 * @brief   This class should be used with the TcpTmTcServer to implement a TCP server
 *          for receiving and sending PUS telemetry and telecommands (TMTC)
 * @details
 * This bridge tasks takes care of filling a FIFO which generated telemetry. The TcpTmTcServer
 * will take care of sending the telemetry stored in the FIFO if a client connects to the
 * server. This bridge will also be the default destination for telecommands, but the telecommands
 * will be relayed to a specified tcDestination directly.
 */
class TcpTmTcBridge : public TmTcBridge {
  friend class TcpTmTcServer;

 public:
  /**
   * Constructor
   * @param objectId  Object ID of the TcpTmTcBridge.
   * @param tcDestination Destination for received TC packets. Any received telecommands will
   *                      be sent there directly. The destination object needs to implement
   *                      AcceptsTelecommandsIF.
   * @param tmStoreId TM store object ID. It is recommended to the default object ID
   * @param tcStoreId TC store object ID. It is recommended to the default object ID
   */
  TcpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
                object_id_t tmStoreId = objects::TM_STORE,
                object_id_t tcStoreId = objects::TC_STORE);
  virtual ~TcpTmTcBridge();

  /**
   * Set properties of internal mutex.
   */
  void setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs);

  ReturnValue_t initialize() override;

 protected:
  ReturnValue_t handleTm() override;
  virtual ReturnValue_t sendTm(const uint8_t* data, size_t dataLen) override;

 private:
  //! Access to the FIFO needs to be mutex protected because it is used by the bridge and
  //! the server.
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  dur_millis_t mutexTimeoutMs = 20;
  MutexIF* mutex;
};

#endif /* FSFW_OSAL_COMMON_TCPTMTCBRIDGE_H_ */
