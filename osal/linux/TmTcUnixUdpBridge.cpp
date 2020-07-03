#include <framework/osal/linux/TmTcUnixUdpBridge.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

TmTcUnixUdpBridge::TmTcUnixUdpBridge(object_id_t objectId,
		object_id_t ccsdsPacketDistributor):
		TmTcBridge(objectId, ccsdsPacketDistributor) {
	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket < 0) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not open"
				" UDP socket!" << std::endl;
		// check errno here.
		return;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(UDP_SERVER_PORT);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &serverSocketOptions,
			sizeof(serverSocketOptions));

	int result = bind(serverSocket,
			reinterpret_cast<struct sockaddr*>(&serverAddress),
			sizeof(serverAddress));
	if(result== -1) {
		sif::error << "TmTcUnixUdpBridge::TmTcUnixUdpBridge: Could not bind "
				"local port " << UDP_SERVER_PORT << " to server socket!"
				<< std::endl;
		// check errno here.
		return;
	}



}

TmTcUnixUdpBridge::~TmTcUnixUdpBridge() {
}
