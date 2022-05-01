#ifndef FSFW_OSAL_COMMON_UDPTCPOLLINGTASK_H_
#define FSFW_OSAL_COMMON_UDPTCPOLLINGTASK_H_

#include <vector>

#include "../../objectmanager/SystemObject.h"
#include "../../storagemanager/StorageManagerIF.h"
#include "../../tasks/ExecutableObjectIF.h"
#include "UdpTmTcBridge.h"

/**
 * @brief   This class can be used with the UdpTmTcBridge to implement a UDP server
 *          for receiving and sending PUS TMTC.
 * @details
 * This task is exclusively used to poll telecommands from a given socket and transfer them
 * to the FSFW software bus. It used the blocking recvfrom call to do this.
 */
class UdpTcPollingTask : public TcpIpBase, public SystemObject, public ExecutableObjectIF {
  friend class TmTcWinUdpBridge;

 public:
  static constexpr size_t DEFAULT_MAX_RECV_SIZE = 1500;
  //! 0.5  default milliseconds timeout for now.
  static const timeval DEFAULT_TIMEOUT;

  UdpTcPollingTask(object_id_t objectId, object_id_t tmtcUdpBridge, size_t maxRecvSize = 0,
                   double timeoutSeconds = -1);
  ~UdpTcPollingTask() override = default;

  /**
   * Turn on optional timeout for UDP polling. In the default mode,
   * the receive function will block until a packet is received.
   * @param timeoutSeconds
   */
  void setTimeout(double timeoutSeconds);

  [[noreturn]] ReturnValue_t performOperation(uint8_t opCode) override;
  ReturnValue_t initialize() override;
  ReturnValue_t initializeAfterTaskCreation() override;

 protected:
  StorageManagerIF* tcStore = nullptr;

 private:
  //! TMTC bridge is cached.
  object_id_t tmtcBridgeId = objects::NO_OBJECT;
  UdpTmTcBridge* tmtcBridge = nullptr;
  MessageQueueId_t targetTcDestination = MessageQueueIF::NO_QUEUE;
  int receptionFlags = 0;

  std::vector<uint8_t> receptionBuffer;

  size_t frameSize = 0;
  timeval receptionTimeout{};

  ReturnValue_t handleSuccessfullTcRead(size_t bytesRead);
};

#endif /* FSFW_OSAL_COMMON_UDPTCPOLLINGTASK_H_ */
