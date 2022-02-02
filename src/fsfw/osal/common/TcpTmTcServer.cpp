#include "TcpTmTcServer.h"

#include "TcpTmTcBridge.h"
#include "fsfw/FSFW.h"
#include "fsfw/container/SharedRingBuffer.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/TaskFactory.h"
#include "fsfw/tmtcservices/SpacePacketParser.h"
#include "fsfw/tmtcservices/TmTcMessage.h"
#include "tcpipHelpers.h"

#ifdef PLATFORM_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(PLATFORM_UNIX)
#include <netdb.h>
#endif

const std::string TcpTmTcServer::DEFAULT_SERVER_PORT = tcpip::DEFAULT_SERVER_PORT;

TcpTmTcServer::TcpTmTcServer(object_id_t objectId, object_id_t tmtcTcpBridge,
                             size_t receptionBufferSize, size_t ringBufferSize,
                             std::string customTcpServerPort, ReceptionModes receptionMode)
    : SystemObject(objectId),
      tmtcBridgeId(tmtcTcpBridge),
      receptionMode(receptionMode),
      tcpConfig(customTcpServerPort),
      receptionBuffer(receptionBufferSize),
      ringBuffer(ringBufferSize, true) {}

ReturnValue_t TcpTmTcServer::initialize() {
  using namespace tcpip;

  ReturnValue_t result = TcpIpBase::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  switch (receptionMode) {
    case (ReceptionModes::SPACE_PACKETS): {
      spacePacketParser = new SpacePacketParser(validPacketIds);
      if (spacePacketParser == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
#if defined PLATFORM_UNIX
      tcpConfig.tcpFlags |= MSG_DONTWAIT;
#endif
    }
  }
  tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
  if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TcpTmTcServer::initialize: TC store uninitialized!" << std::endl;
#else
    sif::printError("TcpTmTcServer::initialize: TC store uninitialized!\n");
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  tmtcBridge = ObjectManager::instance()->get<TcpTmTcBridge>(tmtcBridgeId);

  int retval = 0;
  struct addrinfo* addrResult = nullptr;
  struct addrinfo hints = {};

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Listen to all addresses (0.0.0.0) by using AI_PASSIVE in the hint flags
  retval = getaddrinfo(nullptr, tcpConfig.tcpPort.c_str(), &hints, &addrResult);
  if (retval != 0) {
    handleError(Protocol::TCP, ErrorSources::GETADDRINFO_CALL);
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  // Open TCP (stream) socket
  listenerTcpSocket =
      socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
  if (listenerTcpSocket == INVALID_SOCKET) {
    freeaddrinfo(addrResult);
    handleError(Protocol::TCP, ErrorSources::SOCKET_CALL);
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  // Bind to the address found by getaddrinfo
  retval = bind(listenerTcpSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
  if (retval == SOCKET_ERROR) {
    freeaddrinfo(addrResult);
    handleError(Protocol::TCP, ErrorSources::BIND_CALL);
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  freeaddrinfo(addrResult);
  return HasReturnvaluesIF::RETURN_OK;
}

TcpTmTcServer::~TcpTmTcServer() { closeSocket(listenerTcpSocket); }

ReturnValue_t TcpTmTcServer::performOperation(uint8_t opCode) {
  using namespace tcpip;
  // If a connection is accepted, the corresponding socket will be assigned to the new socket
  socket_t connSocket = 0;
  // sockaddr clientSockAddr = {};
  // socklen_t connectorSockAddrLen = 0;
  int retval = 0;

  // Listen for connection requests permanently for lifetime of program
  while (true) {
    retval = listen(listenerTcpSocket, tcpConfig.tcpBacklog);
    if (retval == SOCKET_ERROR) {
      handleError(Protocol::TCP, ErrorSources::LISTEN_CALL, 500);
      continue;
    }

    // connSocket = accept(listenerTcpSocket, &clientSockAddr, &connectorSockAddrLen);
    connSocket = accept(listenerTcpSocket, nullptr, nullptr);

    if (connSocket == INVALID_SOCKET) {
      handleError(Protocol::TCP, ErrorSources::ACCEPT_CALL, 500);
      closeSocket(connSocket);
      continue;
    };

    handleServerOperation(connSocket);

    // Done, shut down connection and go back to listening for client requests
    retval = shutdown(connSocket, SHUT_BOTH);
    if (retval != 0) {
      handleError(Protocol::TCP, ErrorSources::SHUTDOWN_CALL);
    }
    closeSocket(connSocket);
    connSocket = 0;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcpTmTcServer::initializeAfterTaskCreation() {
  if (tmtcBridge == nullptr) {
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  /* Initialize the destination after task creation. This ensures
  that the destination has already been set in the TMTC bridge. */
  targetTcDestination = tmtcBridge->getRequestQueue();
  tcStore = tmtcBridge->tcStore;
  tmStore = tmtcBridge->tmStore;
  return HasReturnvaluesIF::RETURN_OK;
}

void TcpTmTcServer::handleServerOperation(socket_t& connSocket) {
#if defined PLATFORM_WIN
  setSocketNonBlocking(connSocket);
#endif

  while (true) {
    int retval = recv(connSocket, reinterpret_cast<char*>(receptionBuffer.data()),
                      receptionBuffer.capacity(), tcpConfig.tcpFlags);
    if (retval == 0) {
      size_t availableReadData = ringBuffer.getAvailableReadData();
      if (availableReadData > lastRingBufferSize) {
        handleTcRingBufferData(availableReadData);
      }
      return;
    } else if (retval > 0) {
      // The ring buffer was configured for overwrite, so the returnvalue does not need to
      // be checked for now
      ringBuffer.writeData(receptionBuffer.data(), retval);
    } else if (retval < 0) {
      int errorValue = getLastSocketError();
#if defined PLATFORM_UNIX
      int wouldBlockValue = EAGAIN;
#elif defined PLATFORM_WIN
      int wouldBlockValue = WSAEWOULDBLOCK;
#endif
      if (errorValue == wouldBlockValue) {
        // No data available. Check whether any packets have been read, then send back
        // telemetry if available
        bool tcAvailable = false;
        bool tmSent = false;
        size_t availableReadData = ringBuffer.getAvailableReadData();
        if (availableReadData > lastRingBufferSize) {
          tcAvailable = true;
          handleTcRingBufferData(availableReadData);
        }
        ReturnValue_t result = handleTmSending(connSocket, tmSent);
        if (result == CONN_BROKEN) {
          return;
        }
        if (not tcAvailable and not tmSent) {
          TaskFactory::delayTask(tcpConfig.tcpLoopDelay);
        }
      } else {
        tcpip::handleError(tcpip::Protocol::TCP, tcpip::ErrorSources::RECV_CALL, 300);
      }
    }
  }
}

ReturnValue_t TcpTmTcServer::handleTcReception(uint8_t* spacePacket, size_t packetSize) {
  if (wiretappingEnabled) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Received TC:" << std::endl;
#else
    sif::printInfo("Received TC:\n");
#endif
    arrayprinter::print(spacePacket, packetSize);
  }

  if (spacePacket == nullptr or packetSize == 0) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  store_address_t storeId;
  ReturnValue_t result = tcStore->addData(&storeId, spacePacket, packetSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TcpTmTcServer::handleServerOperation: Data storage with packet size"
                 << packetSize << " failed" << std::endl;
#else
    sif::printWarning(
        "TcpTmTcServer::handleServerOperation: Data storage with packet size %d "
        "failed\n",
        packetSize);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return result;
  }

  TmTcMessage message(storeId);

  result = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TcpTmTcServer::handleServerOperation: "
                    " Sending message to queue failed"
                 << std::endl;
#else
    sif::printWarning(
        "TcpTmTcServer::handleServerOperation: "
        " Sending message to queue failed\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    tcStore->deleteData(storeId);
  }
  return result;
}

std::string TcpTmTcServer::getTcpPort() const { return tcpConfig.tcpPort; }

void TcpTmTcServer::setSpacePacketParsingOptions(std::vector<uint16_t> validPacketIds) {
  this->validPacketIds = validPacketIds;
}

TcpTmTcServer::TcpConfig& TcpTmTcServer::getTcpConfigStruct() { return tcpConfig; }

ReturnValue_t TcpTmTcServer::handleTmSending(socket_t connSocket, bool& tmSent) {
  // Access to the FIFO is mutex protected because it is filled by the bridge
  MutexGuard(tmtcBridge->mutex, tmtcBridge->timeoutType, tmtcBridge->mutexTimeoutMs);
  store_address_t storeId;
  while ((not tmtcBridge->tmFifo->empty()) and
         (tmtcBridge->packetSentCounter < tmtcBridge->sentPacketsPerCycle)) {
    // Send can fail, so only peek from the FIFO
    tmtcBridge->tmFifo->peek(&storeId);

    // Using the store accessor will take care of deleting TM from the store automatically
    ConstStorageAccessor storeAccessor(storeId);
    ReturnValue_t result = tmStore->getData(storeId, storeAccessor);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      return result;
    }
    if (wiretappingEnabled) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::info << "Sending TM:" << std::endl;
#else
      sif::printInfo("Sending TM:\n");
#endif
      arrayprinter::print(storeAccessor.data(), storeAccessor.size());
    }
    int retval = send(connSocket, reinterpret_cast<const char*>(storeAccessor.data()),
                      storeAccessor.size(), tcpConfig.tcpTmFlags);
    if (retval == static_cast<int>(storeAccessor.size())) {
      // Packet sent, clear FIFO entry
      tmtcBridge->tmFifo->pop();
      tmSent = true;

    } else if (retval <= 0) {
      // Assume that the client has closed the connection here for now
      handleSocketError(storeAccessor);
      return CONN_BROKEN;
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcpTmTcServer::handleTcRingBufferData(size_t availableReadData) {
  ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  size_t readAmount = availableReadData;
  lastRingBufferSize = availableReadData;
  if (readAmount >= ringBuffer.getMaxSize()) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // Possible configuration error, too much data or/and data coming in too fast,
    // requiring larger buffers
    sif::warning << "TcpTmTcServer::handleServerOperation: Ring buffer reached "
                 << "fill count" << std::endl;
#else
    sif::printWarning(
        "TcpTmTcServer::handleServerOperation: Ring buffer reached "
        "fill count");
#endif
#endif
  }
  if (readAmount >= receptionBuffer.size()) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // Possible configuration error, too much data or/and data coming in too fast,
    // requiring larger buffers
    sif::warning << "TcpTmTcServer::handleServerOperation: "
                    "Reception buffer too small "
                 << std::endl;
#else
    sif::printWarning("TcpTmTcServer::handleServerOperation: Reception buffer too small\n");
#endif
#endif
    readAmount = receptionBuffer.size();
  }
  ringBuffer.readData(receptionBuffer.data(), readAmount, true);
  const uint8_t* bufPtr = receptionBuffer.data();
  const uint8_t** bufPtrPtr = &bufPtr;
  size_t startIdx = 0;
  size_t foundSize = 0;
  size_t readLen = 0;
  while (readLen < readAmount) {
    result =
        spacePacketParser->parseSpacePackets(bufPtrPtr, readAmount, startIdx, foundSize, readLen);
    switch (result) {
      case (SpacePacketParser::NO_PACKET_FOUND):
      case (SpacePacketParser::SPLIT_PACKET): {
        break;
      }
      case (HasReturnvaluesIF::RETURN_OK): {
        result = handleTcReception(receptionBuffer.data() + startIdx, foundSize);
        if (result != HasReturnvaluesIF::RETURN_OK) {
          status = result;
        }
      }
    }
    ringBuffer.deleteData(foundSize);
    lastRingBufferSize = ringBuffer.getAvailableReadData();
    std::memset(receptionBuffer.data() + startIdx, 0, foundSize);
  }
  return status;
}

void TcpTmTcServer::enableWiretapping(bool enable) { this->wiretappingEnabled = enable; }

void TcpTmTcServer::handleSocketError(ConstStorageAccessor& accessor) {
  // Don't delete data
  accessor.release();
  auto socketError = getLastSocketError();
  switch (socketError) {
#if defined PLATFORM_WIN
    case (WSAECONNRESET): {
      // See https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
      // Remote client might have shut down connection
      return;
    }
#else
    case (EPIPE): {
      // See https://man7.org/linux/man-pages/man2/send.2.html
      // Remote client might have shut down connection
      return;
    }
#endif
    default: {
      tcpip::handleError(tcpip::Protocol::TCP, tcpip::ErrorSources::SEND_CALL);
    }
  }
}

#if defined PLATFORM_WIN
void TcpTmTcServer::setSocketNonBlocking(socket_t& connSocket) {
  u_long iMode = 1;
  int iResult = ioctlsocket(connSocket, FIONBIO, &iMode);
  if (iResult != NO_ERROR) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TcpTmTcServer::handleServerOperation: Setting socket"
                    " non-blocking failed with error "
                 << iResult;
#else
    sif::printWarning(
        "TcpTmTcServer::handleServerOperation: Setting socket"
        " non-blocking failed with error %d\n",
        iResult);
#endif
#endif
  }
}
#endif
