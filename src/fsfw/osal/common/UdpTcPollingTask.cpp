#include "fsfw/osal/common/UdpTcPollingTask.h"

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/osal/common/tcpipHelpers.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#ifdef PLATFORM_WIN
#include <winsock2.h>
#elif defined(PLATFORM_UNIX)
#include <sys/socket.h>
#include <sys/types.h>
#endif

//! Debugging preprocessor define.
#define FSFW_UDP_RECV_WIRETAPPING_ENABLED 0

const timeval UdpTcPollingTask::DEFAULT_TIMEOUT = {0, 500000};

UdpTcPollingTask::UdpTcPollingTask(object_id_t objectId, object_id_t tmtcUdpBridge,
                                   size_t maxRecvSize, double timeoutSeconds)
    : SystemObject(objectId), tmtcBridgeId(tmtcUdpBridge) {
  if (maxRecvSize > 0) {
    this->frameSize = maxRecvSize;
  } else {
    this->frameSize = DEFAULT_MAX_RECV_SIZE;
  }

  /* Set up reception buffer with specified frame size.
      For now, it is assumed that only one frame is held in the buffer! */
  receptionBuffer.reserve(this->frameSize);
  receptionBuffer.resize(this->frameSize);

  if (timeoutSeconds == -1) {
    receptionTimeout = UdpTcPollingTask::DEFAULT_TIMEOUT;
  } else {
    receptionTimeout = timevalOperations::toTimeval(timeoutSeconds);
  }
}

[[noreturn]] ReturnValue_t UdpTcPollingTask::performOperation(uint8_t opCode) {
  /* Sender Address is cached here. */
  struct sockaddr senderAddress {};
  socklen_t senderAddressSize = sizeof(senderAddress);

  /* Poll for new UDP datagrams in permanent loop. */
  while (true) {
    ssize_t bytesReceived =
        recvfrom(this->serverSocket, reinterpret_cast<char*>(receptionBuffer.data()), frameSize,
                 receptionFlags, &senderAddress, &senderAddressSize);
    if (bytesReceived == SOCKET_ERROR) {
      /* Handle error */
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "UdpTcPollingTask::performOperation: Reception error." << std::endl;
#endif
      tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::RECVFROM_CALL, 1000);
      continue;
    }
#if FSFW_UDP_RECV_WIRETAPPING_ENABLED == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UdpTcPollingTask::performOperation: " << bytesReceived << " bytes received"
               << std::endl;
#else
#endif
#endif /* FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1 */

    ReturnValue_t result = handleSuccessfullTcRead(bytesReceived);
    if (result != returnvalue::FAILED) {
    }
    tmtcBridge->checkAndSetClientAddress(senderAddress);
  }
}

ReturnValue_t UdpTcPollingTask::handleSuccessfullTcRead(size_t bytesRead) {
  store_address_t storeId;

#if FSFW_UDP_RECV_WIRETAPPING_ENABLED == 1
  arrayprinter::print(receptionBuffer.data(), bytesRead);
#endif

  ReturnValue_t result = tcStore->addData(&storeId, receptionBuffer.data(), bytesRead);
  if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UdpTcPollingTask::transferPusToSoftwareBus: Data storage failed." << std::endl;
    sif::warning << "Packet size: " << bytesRead << std::endl;
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return returnvalue::FAILED;
  }

  TmTcMessage message(storeId);

  result = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
  if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UdpTcPollingTask::handleSuccessfullTcRead: "
                    " Sending message to queue failed"
                 << std::endl;
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    tcStore->deleteData(storeId);
  }
  return result;
}

ReturnValue_t UdpTcPollingTask::initialize() {
  tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
  if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "UdpTcPollingTask::initialize: TC store uninitialized!" << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  tmtcBridge = ObjectManager::instance()->get<UdpTmTcBridge>(tmtcBridgeId);
  if (tmtcBridge == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "UdpTcPollingTask::initialize: Invalid TMTC bridge object!" << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  ReturnValue_t result = TcpIpBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  return returnvalue::OK;
}

ReturnValue_t UdpTcPollingTask::initializeAfterTaskCreation() {
  /* Initialize the destination after task creation. This ensures
      that the destination has already been set in the TMTC bridge. */
  targetTcDestination = tmtcBridge->getRequestQueue();
  /* The server socket is set up in the bridge intialization. Calling this function here
      ensures that it is set up regardless of which class was initialized first */
  this->serverSocket = tmtcBridge->serverSocket;
  return returnvalue::OK;
}

void UdpTcPollingTask::setTimeout(double timeoutSeconds) {
#ifdef PLATFORM_WIN
  DWORD timeoutMs = timeoutSeconds * 1000.0;
  int result = setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO,
                          reinterpret_cast<const char*>(&timeoutMs), sizeof(DWORD));
  if (result == -1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TcWinUdpPollingTask::TcSocketPollingTask: Setting "
                  "receive timeout failed with "
               << strerror(errno) << std::endl;
#endif
  }
#elif defined(PLATFORM_UNIX)
  timeval tval{};
  tval = timevalOperations::toTimeval(timeoutSeconds);
  int result = setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof(receptionTimeout));
  if (result == -1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TcSocketPollingTask::TcSocketPollingTask: Setting "
                  "receive timeout failed with "
               << strerror(errno) << std::endl;
#endif
  }
#endif
}
