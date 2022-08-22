#include "fsfw/osal/common/TcpTmTcBridge.h"

#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/osal/common/TcpTmTcBridge.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#ifdef PLATFORM_WIN

#include <ws2tcpip.h>

#elif defined PLATFORM_UNIX

#include <arpa/inet.h>
#include <netdb.h>

#endif

TcpTmTcBridge::TcpTmTcBridge(object_id_t objectId, object_id_t tcDestination, object_id_t tmStoreId,
                             object_id_t tcStoreId)
    : TmTcBridge("TCP TMTC Bridge", objectId, tcDestination, tmStoreId, tcStoreId) {
  mutex = MutexFactory::instance()->createMutex();
  // Connection is always up, TM is requested by connecting to server and receiving packets
  registerCommConnect();
}

ReturnValue_t TcpTmTcBridge::initialize() {
  ReturnValue_t result = TmTcBridge::initialize();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TcpTmTcBridge::initialize: TmTcBridge initialization failed!" << std::endl;
#else
    sif::printError("TcpTmTcBridge::initialize: TmTcBridge initialization failed!\n");
#endif
    return result;
  }

  return returnvalue::OK;
}

TcpTmTcBridge::~TcpTmTcBridge() {
  if (mutex != nullptr) {
    MutexFactory::instance()->deleteMutex(mutex);
  }
}

ReturnValue_t TcpTmTcBridge::handleTm() {
  // Simply store the telemetry in the FIFO, the server will use it to access the TM
  MutexGuard guard(mutex, timeoutType, mutexTimeoutMs);
  TmTcMessage message;
  ReturnValue_t status = returnvalue::OK;
  for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
       result == returnvalue::OK; result = tmTcReceptionQueue->receiveMessage(&message)) {
    status = storeDownlinkData(&message);
    if (status != returnvalue::OK) {
      break;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t TcpTmTcBridge::sendTm(const uint8_t *data, size_t dataLen) {
  // Not used. The Server uses the FIFO to access and send the telemetry.
  return returnvalue::OK;
}

void TcpTmTcBridge::setMutexProperties(MutexIF::TimeoutType timeoutType, dur_millis_t timeoutMs) {
  this->timeoutType = timeoutType;
  this->mutexTimeoutMs = timeoutMs;
}
