#include "TmTcUnixUdpBridge.h"
#include "tcpipHelpers.h"
#include "../../serviceinterface/ServiceInterface.h"
#include "../../ipc/MutexGuard.h"

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <cstring>

//! Debugging preprocessor define.
#define FSFW_UDP_RCV_WIRETAPPING_ENABLED    1

const std::string TmTcUnixUdpBridge::DEFAULT_UDP_SERVER_PORT =  "7301";
const std::string TmTcUnixUdpBridge::DEFAULT_UDP_CLIENT_PORT =  "7302";

TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId,
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

ReturnValue_t TmTcUnixUdpBridge::initialize() {
    using namespace tcpip;

    ReturnValue_t result = TmTcBridge::initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUnixUdpBridge::initialize: TmTcBridge initialization failed!"
                << std::endl;
#endif
        return result;
    }

    struct addrinfo *addrResult = nullptr;
    struct addrinfo hints;

    std::memset(hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    /* Set up UDP socket:
    https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
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

    /* Set up UDP socket: https://man7.org/linux/man-pages/man7/ip.7.html */
    serverSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if(serverSocket < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open UDP socket!" <<
                std::endl;
#else
        sif::printError("TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open UDP socket!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        freeaddrinfo(addrResult);
        handleError(Protocol::UDP, ErrorSources::SOCKET_CALL);
        handleSocketError();
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    serverAddress.sin_family = AF_INET;

    // Accept packets from any interface.
    //serverAddress.sin_addr.s_addr = inet_addr("127.73.73.0");
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(setServerPort);
    serverAddressLen = sizeof(serverAddress);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &serverSocketOptions,
            sizeof(serverSocketOptions));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddress.sin_port = htons(setClientPort);
    clientAddressLen = sizeof(clientAddress);

    int result = bind(serverSocket,
            reinterpret_cast<struct sockaddr*>(&serverAddress),
            serverAddressLen);
    if(result == -1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
                "local port " << setServerPort << " to server socket!"
                << std::endl;
#endif
        handleBindError();
        return;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
    if(mutex != nullptr) {
        MutexFactory::instance()->deleteMutex(mutex);
    }
    close(serverSocket);
}

ReturnValue_t TmTcUnixUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
	int flags = 0;

	MutexGuard lock(mutex, MutexIF::TimeoutType::WAITING, 10);

	if(ipAddrAnySet){
		clientAddress.sin_addr.s_addr = htons(INADDR_ANY);
		//clientAddress.sin_addr.s_addr = inet_addr("127.73.73.1");
		clientAddressLen = sizeof(serverAddress);
	}

//	char ipAddress [15];
#if FSFW_CPP_OSTREAM_ENABLED == 1
//	sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//					&clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif

	ssize_t bytesSent = sendto(serverSocket, data, dataLen, flags,
			reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLen);
	if(bytesSent < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixUdpBridge::sendTm: Send operation failed."
				<< std::endl;
#endif
		handleSendError();
	}
#if FSFW_CPP_OSTREAM_ENABLED == 1
//	sif::debug << "TmTcUnixUdpBridge::sendTm: " << bytesSent << " bytes were"
//			" sent." << std::endl;
#endif
	return HasReturnvaluesIF::RETURN_OK;
}

void TmTcUnixUdpBridge::checkAndSetClientAddress(sockaddr_in& newAddress) {
	MutexGuard lock(mutex, MutexIF::TimeoutType::WAITING, 10);

//	char ipAddress [15];
#if FSFW_CPP_OSTREAM_ENABLED == 1
//	sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//			&newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
//	sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
//			&clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
#endif

	// Set new IP address if it has changed.
	if(clientAddress.sin_addr.s_addr != newAddress.sin_addr.s_addr) {
		clientAddress.sin_addr.s_addr = newAddress.sin_addr.s_addr;
		clientAddressLen = sizeof(clientAddress);
	}
}


void TmTcUnixUdpBridge::handleSocketError() {
	// See: https://man7.org/linux/man-pages/man2/socket.2.html
	switch(errno) {
	case(EACCES):
	case(EINVAL):
	case(EMFILE):
	case(ENFILE):
	case(EAFNOSUPPORT):
	case(ENOBUFS):
	case(ENOMEM):
	case(EPROTONOSUPPORT):
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleSocketError: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
#endif
		break;
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleSocketError: Unknown error"
				<< std::endl;
#endif
		break;
	}
}

void TmTcUnixUdpBridge::handleBindError() {
	// See: https://man7.org/linux/man-pages/man2/bind.2.html
	switch(errno) {
	case(EACCES): {
		/*
		 Ephermeral ports can be shown with following command:
		 sysctl -A | grep ip_local_port_range
		 */
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleBindError: Port access issue."
				"Ports 1-1024 are reserved on UNIX systems and require root "
				"rights while ephermeral ports should not be used as well."
				<< std::endl;
#endif
	}
	break;
	case(EADDRINUSE):
	case(EBADF):
	case(EINVAL):
	case(ENOTSOCK):
	case(EADDRNOTAVAIL):
	case(EFAULT):
	case(ELOOP):
	case(ENAMETOOLONG):
	case(ENOENT):
	case(ENOMEM):
	case(ENOTDIR):
	case(EROFS): {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleBindError: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
#endif
		break;
	}
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleBindError: Unknown error"
				<< std::endl;
#endif
		break;
	}
}

void TmTcUnixUdpBridge::handleSendError() {
	switch(errno) {
	default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcUnixBridge::handleSendError: "
		        << strerror(errno) << std::endl;
#else
		sif::printError("TmTcUnixBridge::handleSendError: %s\n",
				strerror(errno));
#endif
	}
	}
}

void TmTcUnixUdpBridge::setClientAddressToAny(bool ipAddrAnySet){
	this->ipAddrAnySet = ipAddrAnySet;
}

