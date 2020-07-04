#include <framework/osal/linux/TmTcUnixUdpBridge.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId,
		object_id_t ccsdsPacketDistributor, uint16_t serverPort,
		uint16_t clientPort):
		TmTcBridge(objectId, ccsdsPacketDistributor) {
	uint16_t setServerPort = DEFAULT_UDP_SERVER_PORT;
	if(serverPort != 0xFFFF) {
		setServerPort = serverPort;
	}

	uint16_t setClientPort = DEFAULT_UDP_CLIENT_PORT;
	if(clientPort != 0xFFFF) {
		setClientPort = clientPort;
	}

	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket < 0) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open"
				" UDP socket!" << std::endl;
		// check errno here.
		return;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(setServerPort);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &serverSocketOptions,
			sizeof(serverSocketOptions));

	int result = bind(serverSocket,
			reinterpret_cast<struct sockaddr*>(&serverAddress),
			sizeof(serverAddress));
	if(result == -1) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
				"local port " << setServerPort << " to server socket!"
				<< std::endl;
		// check errno here.
		return;
	}



}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
}

ReturnValue_t TmTcUnixUdpBridge::handleTc() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmTcUnixUdpBridge::receiveTc(uint8_t **recvBuffer, size_t *size) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmTcUnixUdpBridge::sendTm(const uint8_t *data, size_t dataLen) {
	return HasReturnvaluesIF::RETURN_OK;
}
