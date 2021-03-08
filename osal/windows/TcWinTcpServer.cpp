#include "TcWinTcpServer.h"
#include "../../serviceinterface/ServiceInterface.h"
#include <winsock2.h>


TcWinTcpServer::TcWinTcpServer(object_id_t objectId, object_id_t tmtcUnixUdpBridge,
        uint16_t customTcpServerPort):
        SystemObject(objectId) {
    /* Initiates Winsock DLL. */
    WSAData wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcWinUdpBridge::TmTcWinUdpBridge:"
                "WSAStartup failed with error: " << err << std::endl;
#endif
        return;
    }

    /* Open TCP (stream) socket */
    serverTcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    uint16_t tcpPort = customTcpServerPort;

    if(customTcpServerPort == 0xffff) {
        tcpPort = DEFAULT_TCP_SERVER_PORT;
    }

    if(serverTcpSocket == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Socket creation failed!" << std::endl;
        handleError(ErrorSources::SOCKET_CALL);
#endif
    }

    int retval = setsockopt(serverTcpSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST,
            reinterpret_cast<const char*>(&tcpSockOpt), sizeof(tcpSockOpt));
    if(retval != 0) {
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Setting socket options failed!" <<
                std::endl;
        handleError(ErrorSources::SETSOCKOPT_CALL);
    }
    tcpAddress.sin_family = AF_INET;
    tcpAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpAddress.sin_port = htons(tcpPort);

    retval = bind(serverTcpSocket, reinterpret_cast<const sockaddr*>(&tcpAddress),
            tcpAddrLen);
    if(retval != 0) {
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Binding socket failed!" <<
                std::endl;
        handleError(ErrorSources::BIND_CALL);
    }

}

TcWinTcpServer::~TcWinTcpServer() {
    closesocket(serverTcpSocket);
    WSACleanup();
}

ReturnValue_t TcWinTcpServer::initialize() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcWinTcpServer::performOperation(uint8_t opCode) {
    /* If a connection is accepted, the corresponding scoket will be assigned to the new socket */
    SOCKET connectorSocket;
    sockaddr_in connectorSockAddr;
    int connectorSockAddrLen = 0;
    /* Listen for connection requests permanently for lifetime of program */
    while(true) {
        int retval = listen(serverTcpSocket, backlog);
        if(retval != 0) {
            handleError(ErrorSources::LISTEN_CALL);
        }

        connectorSocket = accept(serverTcpSocket, reinterpret_cast<sockaddr*>(&connectorSockAddr),
                &connectorSockAddrLen);

        if(connectorSocket) {};

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
