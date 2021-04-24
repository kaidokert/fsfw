#include "TmTcUnixUdpBridge.h"
#include "../common/tcpipHelpers.h"
#include "../../serviceinterface/ServiceInterface.h"
#include "../../ipc/MutexGuard.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <cstring>

//! Debugging preprocessor define.
#define FSFW_UDP_SEND_WIRETAPPING_ENABLED   0

const std::string TmTcUnixUdpBridge::DEFAULT_UDP_SERVER_PORT =  tcpip::DEFAULT_SERVER_PORT;

TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId, object_id_t tcDestination,
        object_id_t tmStoreId, object_id_t tcStoreId, std::string udpServerPort):
		TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    if(udpServerPort == "") {
        this->udpServerPort = DEFAULT_UDP_SERVER_PORT;
    }
    else {
        this->udpServerPort = udpServerPort;
    }

    mutex = MutexFactory::instance()->createMutex();
    communicationLinkUp = false;
}

ReturnValue_t TmTcUnixUdpBridge::initialize() {
    using namespace tcpip;

    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUnixUdpBridge::initialize: TmTcBridge initialization failed!"
                << std::endl;
#endif
        return result;
    }

    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    /* Set up UDP socket:
    https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    Passing nullptr as the first parameter and specifying AI_PASSIVE in hints will cause
    getaddrinfo to assign the address 0.0.0.0 (any address) */
    int retval = getaddrinfo(nullptr, udpServerPort.c_str(), &hints, &addrResult);
    if (retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::TmTcWinUdpBridge: Retrieving address info failed!" <<
                std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Set up UDP socket: https://man7.org/linux/man-pages/man7/ip.7.html */
    serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if(serverSocket < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open UDP socket!" <<
                std::endl;
#else
        sif::printError("TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open UDP socket!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        freeaddrinfo(addrResult);
        handleError(Protocol::UDP, ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    retval = bind(serverSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not bind "
                "local port (" << udpServerPort << ") to server socket!" << std::endl;
#endif
        freeaddrinfo(addrResult);
        handleError(Protocol::UDP, ErrorSources::BIND_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
    if(mutex != nullptr) {
        MutexFactory::instance()->deleteMutex(mutex);
    }
    close(serverSocket);
}

ReturnValue_t TmTcUnixUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
    int flags = 0;

    /* The target address can be set by different threads so this lock ensures thread-safety */
    MutexGuard lock(mutex, timeoutType, mutexTimeoutMs);

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<<
            inet_ntop(AF_INET,&clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif

    ssize_t bytesSent = sendto(
            serverSocket,
            data,
            dataLen,
            flags,
            reinterpret_cast<sockaddr*>(&clientAddress),
            clientAddressLen
    );
    if(bytesSent < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcUnixUdpBridge::sendTm: Send operation failed." << std::endl;
#endif
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SENDTO_CALL);
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    sif::debug << "TmTcUnixUdpBridge::sendTm: " << bytesSent << " bytes were"
            " sent." << std::endl;
#endif

    return HasReturnvaluesIF::RETURN_OK;
}

void TmTcUnixUdpBridge::checkAndSetClientAddress(sockaddr_in& newAddress) {
    /* The target address can be set by different threads so this lock ensures thread-safety */
    MutexGuard lock(mutex, timeoutType, mutexTimeoutMs);

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
            &newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
    sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
            &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif
    registerCommConnect();

    /* Set new IP address to reply to. */
    clientAddress = newAddress;
    clientAddressLen = sizeof(clientAddress);
}

void TmTcUnixUdpBridge::setMutexProperties(MutexIF::TimeoutType timeoutType,
        dur_millis_t timeoutMs) {
    this->timeoutType = timeoutType;
    this->mutexTimeoutMs = timeoutMs;
}
