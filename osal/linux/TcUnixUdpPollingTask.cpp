#include <framework/osal/linux/TcUnixUdpPollingTask.h>

TcSocketPollingTask::TcSocketPollingTask(object_id_t objectId,
		object_id_t tmtcUnixUdpBridge, size_t frameSize,
		double timeoutSeconds): SystemObject(objectId),
		tmtcBridgeId(tmtcUnixUdpBridge) {

	if(frameSize > 0) {
		this->frameSize = frameSize;
	}
	else {
		this->frameSize = DEFAULT_MAX_FRAME_SIZE;
	}

	// Set up reception buffer with specified frame size.
	// For now, it is assumed that only one frame is held in the buffer!
	receptionBuffer.reserve(this->frameSize);
	receptionBuffer.resize(this->frameSize);

	if(timeoutSeconds == -1) {
		receptionTimeout = DEFAULT_TIMEOUT;
	}
	else {
		receptionTimeout = timevalOperations::toTimeval(timeoutSeconds);
	}

	// Set receive timeout.
	int result = setsockopt(serverUdpSocket, SOL_SOCKET, SO_RCVTIMEO,
			&receptionTimeout, sizeof(receptionTimeout));
	if(result == -1) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Setting receive"
				"timeout failed with " << strerror(errno) << std::endl;
		return;
	}
}

TcSocketPollingTask::~TcSocketPollingTask() {
}

ReturnValue_t TcSocketPollingTask::performOperation(uint8_t opCode) {
	// Poll for new data permanently. The call will block until the specified
	// length of bytes has been received or a timeout occured.
	while(1) {
		//! Sender Address is cached here.
		struct sockaddr_in senderAddress;
		socklen_t senderSockLen = 0;
		ssize_t bytesReceived = recvfrom(serverUdpSocket,
				receptionBuffer.data(), frameSize, receptionFlags,
				reinterpret_cast<sockaddr*>(&senderAddress), &senderSockLen);
		if(bytesReceived < 0) {
			//handle error
			sif::error << "TcSocketPollingTask::performOperation: recvfrom "
					"failed with " << strerror(errno) << std::endl;
			continue;
		}
		sif::debug << "TcSocketPollingTask::performOperation: " << bytesReceived
				<< " bytes received" << std::endl;

		ReturnValue_t result = handleSuccessfullTcRead();
		tmtcBridge->checkAndSetClientAddress(senderAddress);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcSocketPollingTask::initialize() {
	tmtcBridge = objectManager->get<TmTcUnixUdpBridge>(tmtcBridgeId);
	if(tmtcBridge == nullptr) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Invalid"
				" TMTC bridge object!" << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	serverUdpSocket = tmtcBridge->serverSocket;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcSocketPollingTask::handleSuccessfullTcRead() {
	return HasReturnvaluesIF::RETURN_OK;
}
