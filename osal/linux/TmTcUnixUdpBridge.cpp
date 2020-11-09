#include "TmTcUnixUdpBridge.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../ipc/MutexHelper.h"

#include <errno.h>
#include <arpa/inet.h>


TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId,
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
	if(serverSocket < 0) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open"
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
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
				"local port " << setServerPort << " to server socket!"
				<< std::endl;
		handleBindError();
		return;
	}
}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
}

ReturnValue_t TmTcUnixUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
	int flags = 0;

	MutexHelper lock(mutex, MutexIF::TimeoutType::WAITING, 10);

	if(ipAddrAnySet){
		clientAddress.sin_addr.s_addr = htons(INADDR_ANY);
		//clientAddress.sin_addr.s_addr = inet_addr("127.73.73.1");
		clientAddressLen = sizeof(serverAddress);
	}

//	char ipAddress [15];
//	sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//					&clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;

	ssize_t bytesSent = sendto(serverSocket, data, dataLen, flags,
			reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLen);
	if(bytesSent < 0) {
		sif::error << "TmTcUnixUdpBridge::sendTm: Send operation failed."
				<< std::endl;
		handleSendError();
	}
//	sif::debug << "TmTcUnixUdpBridge::sendTm: " << bytesSent << " bytes were"
//			" sent." << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

void TmTcUnixUdpBridge::checkAndSetClientAddress(sockaddr_in& newAddress) {
	MutexHelper lock(mutex, MutexIF::TimeoutType::WAITING, 10);

//	char ipAddress [15];
//	sif::debug << "IP Address Sender: "<< inet_ntop(AF_INET,
//			&newAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;
//	sif::debug << "IP Address Old: " <<  inet_ntop(AF_INET,
//			&clientAddress.sin_addr.s_addr, ipAddress, 15) << std::endl;

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
		sif::error << "TmTcUnixBridge::handleSocketError: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
		break;
	default:
		sif::error << "TmTcUnixBridge::handleSocketError: Unknown error"
				<< std::endl;
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
		sif::error << "TmTcUnixBridge::handleBindError: Port access issue."
				"Ports 1-1024 are reserved on UNIX systems and require root "
				"rights while ephermeral ports should not be used as well."
				<< std::endl;
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
		sif::error << "TmTcUnixBridge::handleBindError: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
		break;
	}
	default:
		sif::error << "TmTcUnixBridge::handleBindError: Unknown error"
				<< std::endl;
		break;
	}
}

void TmTcUnixUdpBridge::handleSendError() {
	switch(errno) {
	default:
		sif::error << "TmTcUnixBridge::handleSendError: "
		        << strerror(errno) << std::endl;
	}
}

void TmTcUnixUdpBridge::setClientAddressToAny(bool ipAddrAnySet){
	this->ipAddrAnySet = ipAddrAnySet;
}

