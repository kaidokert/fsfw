#include <framework/osal/linux/TmTcUnixUdpBridge.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <errno.h>
#include <framework/ipc/MutexHelper.h>

TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId,
		object_id_t ccsdsPacketDistributor, uint16_t serverPort,
		uint16_t clientPort):
		TmTcBridge(objectId, ccsdsPacketDistributor) {
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
	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket < 0) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open"
				" UDP socket!" << std::endl;
		// check errno here.
		handleSocketError();
		return;
	}

	serverAddress.sin_family = AF_INET;
	// Accept packets from any interface.
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(setServerPort);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &serverSocketOptions,
			sizeof(serverSocketOptions));

	serverSocketLen = sizeof(serverAddress);
	int result = bind(serverSocket,
			reinterpret_cast<struct sockaddr*>(&serverAddress),
			serverSocketLen);
	if(result == -1) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
				"local port " << setServerPort << " to server socket!"
				<< std::endl;
		// check errno here.
		handleBindError();
		return;
	}
}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
}

ReturnValue_t TmTcUnixUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
	int flags = 0;
	ssize_t result = send(serverSocket, data, dataLen, flags);
	if(result < 0) {
		//handle error
		sif::error << "TmTcUnixUdpBridge::sendTm: Send operation failed "
				"with error " << strerror(errno) << std::endl;
	}

	return HasReturnvaluesIF::RETURN_OK;
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
		sif::error << "TmTcUnixBridge::TmTcUnixBridge: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
		break;
	default:
		sif::error << "TmTcUnixBridge::TmTcUnixBridge: Unknown error"
				<< std::endl;
		break;
	}
}

void TmTcUnixUdpBridge::setTimeout(float timeoutSeconds) {
}

void TmTcUnixUdpBridge::checkAndSetClientAddress(sockaddr_in newAddress) {
	MutexHelper lock(mutex, 10);
	// Set new IP address if it has changed.
	if(clientAddress.sin_addr.s_addr != newAddress.sin_addr.s_addr) {
		clientAddress.sin_addr.s_addr = newAddress.sin_addr.s_addr;
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
		sif::error << "TmTcUnixBridge::TmTcUnixBridge: Port access issue."
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
		sif::error << "TmTcUnixBridge::TmTcUnixBridge: Socket creation failed"
				<< " with " << strerror(errno) << std::endl;
		break;
	}
	default: {
		sif::error << "TmTcUnixBridge::TmTcUnixBridge: Unknown error"
				<< std::endl;
		break;
	}
	}
}


ReturnValue_t TmTcUnixUdpBridge::receiveTc(uint8_t **recvBuffer, size_t *size) {
	// TC reception handled by separate polling task because it is blocking.
	return HasReturnvaluesIF::RETURN_OK;
}
