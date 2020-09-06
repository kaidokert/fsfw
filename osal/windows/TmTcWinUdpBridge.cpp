#include <fsfw/ipc/MutexHelper.h>
#include "TmTcWinUdpBridge.h"

TmTcWinUdpBridge::TmTcWinUdpBridge(object_id_t objectId,
        object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId,
        uint16_t serverPort, uint16_t clientPort):
        TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    mutex = MutexFactory::instance()->createMutex();

    // Initiates Winsock DLL.
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge:"
                "WSAStartup failed with error: " << err << std::endl;
        return;
    }

    uint16_t setServerPort = DEFAULT_UDP_SERVER_PORT;
    if(serverPort != 0xFFFF) {
        setServerPort = serverPort;
    }

    uint16_t setClientPort = DEFAULT_UDP_CLIENT_PORT;
    if(clientPort != 0xFFFF) {
        setClientPort = clientPort;
    }

    // Set up UDP socket: https://man7.org/linux/man-pages/man7/ip.7.html
    //clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(serverSocket == INVALID_SOCKET) {
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not open"
                " UDP socket!" << std::endl;
        handleSocketError();
        return;
    }

    serverAddress.sin_family = AF_INET;

    // Accept packets from any interface. (potentially insecure).
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(setServerPort);
    serverAddressLen = sizeof(serverAddress);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char*>(&serverSocketOptions),
            sizeof(serverSocketOptions));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddress.sin_port = htons(setClientPort);
    clientAddressLen = sizeof(clientAddress);

    int result = bind(serverSocket,
            reinterpret_cast<struct sockaddr*>(&serverAddress),
            serverAddressLen);
    if(result != 0) {
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not bind "
                "local port " << setServerPort << " to server socket!"
                << std::endl;
        handleBindError();
    }
}

TmTcWinUdpBridge::~TmTcWinUdpBridge() {
    WSACleanup();
}

ReturnValue_t TmTcWinUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
    int flags = 0;

    //clientAddress.sin_addr.s_addr = htons(INADDR_ANY);
    //clientAddressLen = sizeof(serverAddress);

//  char ipAddress [15];
//  sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//                  &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;

    ssize_t bytesSent = sendto(serverSocket,
            reinterpret_cast<const char*>(data), dataLen, flags,
            reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLen);
    if(bytesSent == SOCKET_ERROR) {
        sif::error << "TmTcWinUdpBridge::sendTm: Send operation failed."
                << std::endl;
        handleSendError();
    }
//  sif::debug << "TmTcUnixUdpBridge::sendTm: " << bytesSent << " bytes were"
//          " sent." << std::endl;
    return HasReturnvaluesIF::RETURN_OK;
    return HasReturnvaluesIF::RETURN_OK;
}

void TmTcWinUdpBridge::checkAndSetClientAddress(sockaddr_in newAddress) {
    MutexHelper lock(mutex, MutexIF::TimeoutType::WAITING, 10);

//  char ipAddress [15];
//  sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//          &newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
//  sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
//          &clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;

    // Set new IP address if it has changed.
    if(clientAddress.sin_addr.s_addr != newAddress.sin_addr.s_addr) {
        clientAddress.sin_addr.s_addr = newAddress.sin_addr.s_addr;
        clientAddressLen = sizeof(clientAddress);
    }
}

void TmTcWinUdpBridge::handleSocketError() {
    int errCode = WSAGetLastError();
    switch(errCode) {
    case(WSANOTINITIALISED): {
        sif::info << "TmTcWinUdpBridge::handleSocketError: WSANOTINITIALISED: "
                << "WSAStartup(...) call " << "necessary" << std::endl;
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/
        windows-sockets-error-codes-2
         */
        sif::info << "TmTcWinUdpBridge::handleSocketError: Error code: "
                << errCode << std::endl;
        break;
    }
    }
}

void TmTcWinUdpBridge::handleBindError() {
    int errCode = WSAGetLastError();
    switch(errCode) {
    case(WSANOTINITIALISED): {
        sif::info << "TmTcWinUdpBridge::handleBindError: WSANOTINITIALISED: "
                << "WSAStartup(...) call " << "necessary" << std::endl;
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/
        windows-sockets-error-codes-2
        */
        sif::info << "TmTcWinUdpBridge::handleBindError: Error code: "
                << errCode << std::endl;
        break;
    }
    }
}

void TmTcWinUdpBridge::handleSendError() {
    int errCode = WSAGetLastError();
    switch(errCode) {
    case(WSANOTINITIALISED): {
        sif::info << "TmTcWinUdpBridge::handleSendError: WSANOTINITIALISED: "
                << "WSAStartup(...) call " << "necessary" << std::endl;
        break;
    }
    case(WSAEADDRNOTAVAIL): {
        sif::info << "TmTcWinUdpBridge::handleReadError: WSAEADDRNOTAVAIL: "
                << "Check target address. " << std::endl;
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/
        windows-sockets-error-codes-2
        */
        sif::info << "TmTcWinUdpBridge::handleSendError: Error code: "
                << errCode << std::endl;
        break;
    }
    }
}

