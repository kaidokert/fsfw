#include "TcWinTcpServer.h"
#include "../../serviceinterface/ServiceInterface.h"
#include <winsock2.h>

TcWinTcpServer::TcWinTcpServer(object_id_t objectId, object_id_t tmtcUnixUdpBridge):
        SystemObject(objectId) {
    /* Open TCP socket */
    serverTcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverTcpSocket == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TcWinTcpServer::TcWinTcpServer: Socket creation failed!" << std::endl;
        handleSocketError();
#endif
    }

    setsockopt(serverTcpSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST,
            &tcpSockOpt, sizeof(tcpSockOpt));
}

TcWinTcpServer::~TcWinTcpServer() {
}

void TcWinTcpServer::handleSocketError() {
    int errCode = WSAGetLastError();
    switch(errCode) {
    case(WSANOTINITIALISED): {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::handleSocketError: WSANOTINITIALISED: WSAStartup"
                " call necessary" << std::endl;
#endif
        break;
    }
    default: {
        /*
        https://docs.microsoft.com/en-us/windows/win32/winsock/
        windows-sockets-error-codes-2
         */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TmTcWinUdpBridge::handleSocketError: Error code: " << errCode << std::endl;
#endif
        break;
    }
    }
}


