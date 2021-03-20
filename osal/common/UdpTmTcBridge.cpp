#include "tcpipHelpers.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/MutexGuard.h>
#include <fsfw/osal/common/UdpTmTcBridge.h>

#ifdef _WIN32
#include <ws2tcpip.h>

#elif defined(__unix__)

#include <arap/inet.h>

#endif

//! Debugging preprocessor define.
#define FSFW_UDP_SEND_WIRETAPPING_ENABLED    0

const std::string UdpTmTcBridge::DEFAULT_UDP_SERVER_PORT =  tcpip::DEFAULT_SERVER_PORT;

UdpTmTcBridge::UdpTmTcBridge(object_id_t objectId, object_id_t tcDestination,
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

ReturnValue_t UdpTmTcBridge::initialize() {
    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUdpBridge::initialize: TmTcBridge initialization failed!"
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
        sif::error << "TmTcUdpBridge::TmTcUdpBridge: WSAStartup failed with error: " <<
                err << std::endl;
#else
        sif::printError("TmTcUdpBridge::TmTcUdpBridge: WSAStartup failed with error: %d\n",
                err);
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
#endif

    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints = { 0 };

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
#ifdef _WIN32
    /* See:
    https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
    for information about AI_PASSIVE. */
    hints.ai_flags = AI_PASSIVE;
#endif

    /* Set up UDP socket:
    https://en.wikipedia.org/wiki/Getaddrinfo
    Passing nullptr as the first parameter and specifying AI_PASSIVE in hints will cause
    getaddrinfo to assign the address 0.0.0.0 (any address) */
    int retval = getaddrinfo(nullptr, udpServerPort.c_str(), &hints, &addrResult);
    if (retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcUdpBridge::TmTcUdpBridge: Retrieving address info failed!" <<
                std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if(serverSocket == INVALID_SOCKET) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcUdpBridge::TmTcUdpBridge: Could not open UDP socket!" <<
                std::endl;
#endif
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    retval = bind(serverSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUdpBridge::TmTcUdpBridge: Could not bind "
                "local port (" << udpServerPort << ") to server socket!" << std::endl;
#endif
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::BIND_CALL);
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

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
            &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif

    int bytesSent = sendto(
            serverSocket,
            reinterpret_cast<const char*>(data),
            dataLen,
            flags,
            reinterpret_cast<sockaddr*>(&clientAddress),
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

void UdpTmTcBridge::checkAndSetClientAddress(sockaddr_in& newAddress) {
    /* The target address can be set by different threads so this lock ensures thread-safety */
    MutexGuard lock(mutex, timeoutType, mutexTimeoutMs);

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
            &newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
    sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
            &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
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
