#include "TcWinTcpServer.h"
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
        handleError(ErrorSources::GETADDRINFO_CALL);
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
        handleError(ErrorSources::SOCKET_CALL);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

//    retval = setsockopt(listenerTcpSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST,
//            reinterpret_cast<const char*>(&tcpSockOpt), sizeof(tcpSockOpt));
//    if(retval != 0) {
//        sif::warning << "TcWinTcpServer::TcWinTcpServer: Setting socket options failed!" <<
//                std::endl;
//        handleError(ErrorSources::SETSOCKOPT_CALL);
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//    tcpAddress.sin_family = AF_INET;
//    tcpAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(listenerTcpSocket, reinterpret_cast<const sockaddr*>(&tcpAddress),
            tcpAddrLen);
    if(retval == SOCKET_ERROR) {
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Binding socket failed!" <<
                std::endl;
        freeaddrinfo(addrResult);
        handleError(ErrorSources::BIND_CALL);
    }

    freeaddrinfo(addrResult);
    return HasReturnvaluesIF::RETURN_OK;
}


TcWinTcpServer::~TcWinTcpServer() {
    closesocket(listenerTcpSocket);
    WSACleanup();
}

ReturnValue_t TcWinTcpServer::performOperation(uint8_t opCode) {
    /* If a connection is accepted, the corresponding scoket will be assigned to the new socket */
    SOCKET clientSocket;
    sockaddr_in clientSockAddr;
    int connectorSockAddrLen = 0;
    int retval = 0;
    /* Listen for connection requests permanently for lifetime of program */
    while(true) {
        retval = listen(listenerTcpSocket, currentBacklog);
        if(retval == SOCKET_ERROR) {
            handleError(ErrorSources::LISTEN_CALL);
            continue;
        }

        clientSocket = accept(listenerTcpSocket, reinterpret_cast<sockaddr*>(&clientSockAddr),
                &connectorSockAddrLen);

        if(clientSocket == INVALID_SOCKET) {
            handleError(ErrorSources::ACCEPT_CALL);
            continue;
        };

        retval = recv(clientSocket, reinterpret_cast<char*>(receptionBuffer.data()),
                receptionBuffer.size(), 0);
#if FSFW_TCP_SERVER_WIRETAPPING_ENABLED == 1
#endif

    }
    return HasReturnvaluesIF::RETURN_OK;
}

void TcWinTcpServer::handleError(ErrorSources errorSrc) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    int errCode = WSAGetLastError();
    std::string errorSrcString;
    if(errorSrc == ErrorSources::SETSOCKOPT_CALL) {
        errorSrcString = "setsockopt call";
    }
    else if(errorSrc == ErrorSources::SOCKET_CALL) {
        errorSrcString = "socket call";
    }
    else if(errorSrc == ErrorSources::LISTEN_CALL) {
        errorSrcString = "listen call";
    }
    else if(errorSrc == ErrorSources::ACCEPT_CALL) {
        errorSrcString = "accept call";
    }
    else if(errorSrc == ErrorSources::GETADDRINFO_CALL) {
        errorSrcString = "getaddrinfo call";
    }

    switch(errCode) {
    case(WSANOTINITIALISED): {
        sif::warning << "TmTcWinUdpBridge::handleError: " << errorSrcString << " | "
                "WSANOTINITIALISED: WSAStartup call necessary" << std::endl;
        break;
    }
    case(WSAEINVAL): {
        sif::warning << "TmTcWinUdpBridge::handleError: " << errorSrcString << " | "
                "WSAEINVAL: Invalid parameters" << std::endl;
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/
        windows-sockets-error-codes-2
         */
        sif::warning << "TmTcWinUdpBridge::handleSocketError: Error code: " << errCode << std::endl;
        break;
    }
    }
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
}
