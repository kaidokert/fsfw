#include "TcpTmTcBridge.h"
#include "tcpipHelpers.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/MutexGuard.h>
#include <fsfw/osal/common/TcpTmTcBridge.h>

#ifdef _WIN32

#include <ws2tcpip.h>

#elif defined(__unix__)

#include <netdb.h>
#include <arpa/inet.h>

#endif

const std::string TcpTmTcBridge::DEFAULT_UDP_SERVER_PORT =  tcpip::DEFAULT_SERVER_PORT;

TcpTmTcBridge::TcpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
        object_id_t tmStoreId, object_id_t tcStoreId):
        TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    mutex = MutexFactory::instance()->createMutex();
    communicationLinkUp = false;
}

ReturnValue_t TcpTmTcBridge::initialize() {
    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUdpBridge::initialize: TmTcBridge initialization failed!"
                << std::endl;
#endif
        return result;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

TcpTmTcBridge::~TcpTmTcBridge() {
    if(mutex != nullptr) {
        MutexFactory::instance()->deleteMutex(mutex);
    }
}

ReturnValue_t TcpTmTcBridge::sendTm(const uint8_t *data, size_t dataLen) {

    return HasReturnvaluesIF::RETURN_OK;
}


void TcpTmTcBridge::setMutexProperties(MutexIF::TimeoutType timeoutType,
        dur_millis_t timeoutMs) {
    this->timeoutType = timeoutType;
    this->mutexTimeoutMs = timeoutMs;
}
