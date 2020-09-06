#include "TmTcWinUdpBridge.h"

TmTcWinUdpBridge::TmTcWinUdpBridge(object_id_t objectId,
        object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId,
        uint16_t serverPort, uint16_t clientPort):
        TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId) {
    mutex = MutexFactory::instance()->createMutex();

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
    if(serverSocket != 0) {
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: Could not open"
                " UDP socket!" << std::endl;
        handleSocketError();
        return;
    }

    serverAddress.sin_family = AF_INET;

    // Accept packets from any interface.
    //serverAddress.sin_addr.s_addr = inet_addr("127.73.73.0");
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
    if(result == -1) {
        sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
                "local port " << setServerPort << " to server socket!"
                << std::endl;
        handleBindError();
        return;
    }
}

TmTcWinUdpBridge::~TmTcWinUdpBridge() {}

void TmTcWinUdpBridge::handleSocketError() {
}

void TmTcWinUdpBridge::handleBindError() {
}

ReturnValue_t TmTcWinUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

void TmTcWinUdpBridge::checkAndSetClientAddress(sockaddr_in clientAddress) {
}

void TmTcWinUdpBridge::handleSendError() {
}
