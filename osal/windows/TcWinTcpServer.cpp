#include "TcWinTcpServer.h"
#include "tcpipHelpers.h"
#include "../../serviceinterface/ServiceInterface.h"

#include <winsock2.h>
#include <ws2tcpip.h>

const std::string TcWinTcpServer::DEFAULT_TCP_SERVER_PORT =  "7301";
const std::string TcWinTcpServer::DEFAULT_TCP_CLIENT_PORT =  "7302";

TcWinTcpServer::TcWinTcpServer(object_id_t objectId, object_id_t tmtcUnixUdpBridge,
        std::string customTcpServerPort):
        SystemObject(objectId), tcpPort(customTcpServerPort) {
    if(tcpPort == "") {
        tcpPort = DEFAULT_TCP_SERVER_PORT;
    }
}

ReturnValue_t TcWinTcpServer::initialize() {
    using namespace tcpip;
    int retval = 0;
    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints;
    /* Initiates Winsock DLL. */
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable Winsock DLL. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge: WSAStartup failed with error: " <<
                err << std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    retval = getaddrinfo(nullptr, tcpPort.c_str(), &hints, &addrResult);
    if (retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Retrieving address info failed!" <<
                std::endl;
#endif
        handleError(Protocol::TCP, ErrorSources::GETADDRINFO_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Open TCP (stream) socket */
    listenerTcpSocket = socket(addrResult->ai_family, addrResult->ai_socktype,
            addrResult->ai_protocol);
    if(listenerTcpSocket == INVALID_SOCKET) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Socket creation failed!" << std::endl;
#endif
        freeaddrinfo(addrResult);
        handleError(Protocol::TCP, ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    retval = bind(listenerTcpSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
    if(retval == SOCKET_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Binding socket failed!" <<
                std::endl;
#endif
        freeaddrinfo(addrResult);
        handleError(Protocol::TCP, ErrorSources::BIND_CALL);
    }

    freeaddrinfo(addrResult);
    return HasReturnvaluesIF::RETURN_OK;
}


TcWinTcpServer::~TcWinTcpServer() {
    closesocket(listenerTcpSocket);
    WSACleanup();
}

ReturnValue_t TcWinTcpServer::performOperation(uint8_t opCode) {
    using namespace tcpip;
    /* If a connection is accepted, the corresponding socket will be assigned to the new socket */
    SOCKET clientSocket;
    sockaddr_in clientSockAddr;
    int connectorSockAddrLen = 0;
    int retval = 0;

    /* Listen for connection requests permanently for lifetime of program */
    while(true) {
        retval = listen(listenerTcpSocket, currentBacklog);
        if(retval == SOCKET_ERROR) {
            handleError(Protocol::TCP, ErrorSources::LISTEN_CALL, 500);
            continue;
        }

        clientSocket = accept(listenerTcpSocket, reinterpret_cast<sockaddr*>(&clientSockAddr),
                &connectorSockAddrLen);

        if(clientSocket == INVALID_SOCKET) {
            handleError(Protocol::TCP, ErrorSources::ACCEPT_CALL, 500);
            continue;
        };

        retval = recv(clientSocket, reinterpret_cast<char*>(receptionBuffer.data()),
                receptionBuffer.size(), 0);
        if(retval > 0) {
#if FSFW_TCP_RCV_WIRETAPPING_ENABLED == 1
            sif::info << "TcWinTcpServer::performOperation: Received " << retval << " bytes."
                    std::endl;
#endif
            handleError(Protocol::TCP, ErrorSources::RECV_CALL, 500);
        }
        else if(retval == 0) {

        }
        else {

        }

        /* Done, shut down connection */
        retval = shutdown(clientSocket, SD_SEND);
    }
    return HasReturnvaluesIF::RETURN_OK;
}


