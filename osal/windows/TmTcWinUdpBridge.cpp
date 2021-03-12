#include "TmTcWinUdpBridge.h"
#include "tcpipHelpers.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/MutexGuard.h>
#include <ws2tcpip.h>

const std::string TmTcWinUdpBridge::DEFAULT_UDP_SERVER_PORT =  "7301";
const std::string TmTcWinUdpBridge::DEFAULT_UDP_CLIENT_PORT =  "7302";

TmTcWinUdpBridge::TmTcWinUdpBridge(object_id_t objectId,
        object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId,
        std::string udpServerPort, std::string udpClientPort):
        TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    if(udpServerPort == "") {
         this->udpServerPort = DEFAULT_UDP_SERVER_PORT;
    }
    else {
        this->udpServerPort = udpServerPort;
    }
    if(udpClientPort == "") {
        this->udpClientPort = DEFAULT_UDP_CLIENT_PORT;
    }
    else {
        this->udpClientPort = udpClientPort;
    }

    mutex = MutexFactory::instance()->createMutex();
    communicationLinkUp = false;
}

ReturnValue_t TmTcWinUdpBridge::initialize() {
    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::initialize: TmTcBridge initialization failed!"
                << std::endl;
#endif
        return result;
    }

    /* Initiates Winsock DLL. */
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: WSAStartup failed with error: " <<
                err << std::endl;
#else
        sif::printError("TmTcWinUdpBridge::TmTcWinUdpBridge: WSAStartup failed with error: %d\n",
                err);
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::TmTcWinUdpBridge: Retrieving address info failed!" <<
                std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if(serverSocket == INVALID_SOCKET) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not open UDP socket!" <<
                std::endl;
#endif
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    retval = bind(serverSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not bind "
                "local port " << udpServerPort << " to server socket!" << std::endl;
#endif
        freeaddrinfo(addrResult);
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::BIND_CALL);
    }
    freeaddrinfo(addrResult);
    return HasReturnvaluesIF::RETURN_OK;
}

TmTcWinUdpBridge::~TmTcWinUdpBridge() {
    if(mutex != nullptr) {
        MutexFactory::instance()->deleteMutex(mutex);
    }
    closesocket(serverSocket);
    WSACleanup();
}

ReturnValue_t TmTcWinUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
    MutexGuard lock(mutex, MutexIF::TimeoutType::WAITING, 10);
    int flags = 0;

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    clientAddress.sin_addr.s_addr = htons(INADDR_ANY);
    clientAddressLen = sizeof(serverAddress);
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
            &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif

    int bytesSent = sendto(serverSocket,
            reinterpret_cast<const char*>(data), dataLen, flags,
            reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLen);
    if(bytesSent == SOCKET_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::sendTm: Send operation failed."
                << std::endl;
#endif
        tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::SENDTO_CALL);
    }
#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    sif::debug << "TmTcUnixUdpBridge::sendTm: " << bytesSent << " bytes were"
            " sent." << std::endl;
#endif
    return HasReturnvaluesIF::RETURN_OK;
}

void TmTcWinUdpBridge::checkAndSetClientAddress(sockaddr_in& newAddress) {
    MutexGuard lock(mutex, MutexIF::TimeoutType::WAITING, 10);

#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_SEND_WIRETAPPING_ENABLED == 1
    char ipAddress [15];
    sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
            &newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
    sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
            &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif
    registerCommConnect();

    /* Set new IP address if it has changed. */
    if(clientAddress.sin_addr.s_addr != newAddress.sin_addr.s_addr) {
        clientAddress = newAddress;
        clientAddressLen = sizeof(clientAddress);
    }
}

