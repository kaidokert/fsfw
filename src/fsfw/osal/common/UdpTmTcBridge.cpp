#include "fsfw/osal/common/UdpTmTcBridge.h"
#include "fsfw/osal/common/tcpipHelpers.h"

#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/ipc/MutexGuard.h"

#ifdef PLATFORM_WIN
#include <ws2tcpip.h>
#elif defined(PLATFORM_UNIX)
#include <netdb.h>
#include <arpa/inet.h>
#endif

//! Debugging preprocessor define.
#ifndef FSFW_UDP_SEND_WIRETAPPING_ENABLED
#define FSFW_UDP_SEND_WIRETAPPING_ENABLED    0
#endif

const std::string UdpTmTcBridge::DEFAULT_SERVER_PORT =  tcpip::DEFAULT_SERVER_PORT;

UdpTmTcBridge::UdpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
        std::string udpServerPort, object_id_t tmStoreId, object_id_t tcStoreId):
        TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    if(udpServerPort == "") {
        this->udpServerPort = DEFAULT_SERVER_PORT;
    }
    else {
        this->udpServerPort = udpServerPort;
    }

    mutex = MutexFactory::instance()->createMutex();
    communicationLinkUp = false;
}

ReturnValue_t UdpTmTcBridge::initialize() {
    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UdpTmTcBridge::initialize: TmTcBridge initialization failed!"
                << std::endl;
#endif
        return result;
    }


#ifdef _WIN32
    /* Initiates Winsock DLL. */
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UdpTmTcBridge::UdpTmTcBridge: WSAStartup failed with error: " <<
                err << std::endl;
#else
        sif::printError("UdpTmTcBridge::UdpTmTcBridge: WSAStartup failed with error: %d\n",
                err);
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
#endif

    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints = {};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    /* Set up UDP socket:
    https://en.wikipedia.org/wiki/Getaddrinfo
    Passing nullptr as the first parameter and specifying AI_PASSIVE in hints will cause
    getaddrinfo to assign the address 0.0.0.0 (any address) */
    int retval = getaddrinfo(nullptr, udpServerPort.c_str(), &hints, &addrResult);
    if (retval != 0) {
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::GETADDRINFO_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if(serverSocket == INVALID_SOCKET) {
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

#if FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    tcpip::printAddress(addrResult->ai_addr);
#endif

    retval = bind(serverSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
    if(retval != 0) {
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::BIND_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    freeaddrinfo(addrResult);
    return HasReturnvaluesIF::RETURN_OK;
}

UdpTmTcBridge::~UdpTmTcBridge() {
    if(mutex != nullptr) {
        MutexFactory::instance()->deleteMutex(mutex);
    }
}

ReturnValue_t UdpTmTcBridge::sendTm(const uint8_t *data, size_t dataLen) {
    int flags = 0;

    /* The target address can be set by different threads so this lock ensures thread-safety */
    MutexGuard lock(mutex, timeoutType, mutexTimeoutMs);

#if FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    tcpip::printAddress(&clientAddress);
#endif

    int bytesSent = sendto(
            serverSocket,
            reinterpret_cast<const char*>(data),
            dataLen,
            flags,
            &clientAddress,
            clientAddressLen
    );
    if(bytesSent == SOCKET_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcUdpBridge::sendTm: Send operation failed." << std::endl;
#endif
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SENDTO_CALL);
    }
#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    sif::debug << "TmTcUdpBridge::sendTm: " << bytesSent << " bytes were"
            " sent." << std::endl;
#endif
    return HasReturnvaluesIF::RETURN_OK;
}

void UdpTmTcBridge::checkAndSetClientAddress(sockaddr& newAddress) {
    /* The target address can be set by different threads so this lock ensures thread-safety */
    MutexGuard lock(mutex, timeoutType, mutexTimeoutMs);

#if FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    tcpip::printAddress(&newAddress);
    tcpip::printAddress(&clientAddress);
#endif

    registerCommConnect();

    /* Set new IP address to reply to */
    clientAddress = newAddress;
    clientAddressLen = sizeof(clientAddress);
}

void UdpTmTcBridge::setMutexProperties(MutexIF::TimeoutType timeoutType,
        dur_millis_t timeoutMs) {
    this->timeoutType = timeoutType;
    this->mutexTimeoutMs = timeoutMs;
}
