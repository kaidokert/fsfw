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
        handleError(ErrorSources::SOCKET_CALL);
#endif
    }

    int retval = setsockopt(serverTcpSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST,
            reinterpret_cast<const char*>(&tcpSockOpt), sizeof(tcpSockOpt));
    if(retval != 0) {

    }
}

TcWinTcpServer::~TcWinTcpServer() {
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
