#include "TcUnixUdpPollingTask.h"
#include "../../globalfunctions/arrayprinter.h"

TcUnixUdpPollingTask::TcUnixUdpPollingTask(object_id_t objectId,
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
}

TcUnixUdpPollingTask::~TcUnixUdpPollingTask() {}

ReturnValue_t TcUnixUdpPollingTask::performOperation(uint8_t opCode) {
	// Poll for new UDP datagrams in permanent loop.
	while(1) {
		//! Sender Address is cached here.
		struct sockaddr_in senderAddress;
		socklen_t senderSockLen = sizeof(senderAddress);
		ssize_t bytesReceived = recvfrom(serverUdpSocket,
				receptionBuffer.data(), frameSize, receptionFlags,
				reinterpret_cast<sockaddr*>(&senderAddress), &senderSockLen);
		if(bytesReceived < 0) {
			// handle error
			sif::error << "TcSocketPollingTask::performOperation: Reception"
					"error." << std::endl;
			handleReadError();

			continue;
		}
//		sif::debug << "TcSocketPollingTask::performOperation: " << bytesReceived
//				<< " bytes received" << std::endl;

		ReturnValue_t result = handleSuccessfullTcRead(bytesReceived);
		if(result != HasReturnvaluesIF::RETURN_FAILED) {

		}
		tmtcBridge->registerCommConnect();
		tmtcBridge->checkAndSetClientAddress(senderAddress);
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t TcUnixUdpPollingTask::handleSuccessfullTcRead(size_t bytesRead) {
	store_address_t storeId;
	ReturnValue_t result = tcStore->addData(&storeId,
			receptionBuffer.data(), bytesRead);
	// arrayprinter::print(receptionBuffer.data(), bytesRead);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "TcSerialPollingTask::transferPusToSoftwareBus: Data "
				"storage failed" << std::endl;
		sif::error << "Packet size: " << bytesRead << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	TmTcMessage message(storeId);

	result  = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "Serial Polling: Sending message to queue failed"
				<< std::endl;
		tcStore->deleteData(storeId);
	}
	return result;
}

ReturnValue_t TcUnixUdpPollingTask::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == nullptr) {
		sif::error << "TcSerialPollingTask::initialize: TC Store uninitialized!"
				<< std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	tmtcBridge = objectManager->get<TmTcUnixUdpBridge>(tmtcBridgeId);
	if(tmtcBridge == nullptr) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Invalid"
				" TMTC bridge object!" << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	serverUdpSocket = tmtcBridge->serverSocket;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcUnixUdpPollingTask::initializeAfterTaskCreation() {
	// Initialize the destination after task creation. This ensures
	// that the destination will be set in the TMTC bridge.
	targetTcDestination = tmtcBridge->getRequestQueue();
	return HasReturnvaluesIF::RETURN_OK;
}

void TcUnixUdpPollingTask::setTimeout(double timeoutSeconds) {
	timeval tval;
	tval = timevalOperations::toTimeval(timeoutSeconds);
	int result = setsockopt(serverUdpSocket, SOL_SOCKET, SO_RCVTIMEO,
			&tval, sizeof(receptionTimeout));
	if(result == -1) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Setting "
				"receive timeout failed with " << strerror(errno) << std::endl;
	}
}

// TODO: sleep after error detection to prevent spam
void TcUnixUdpPollingTask::handleReadError() {
	switch(errno) {
	case(EAGAIN): {
		// todo: When working in timeout mode, this will occur more often
		// and is not an error.
		sif::error << "TcUnixUdpPollingTask::handleReadError: Timeout."
				<< std::endl;
		break;
	}
	default: {
		sif::error << "TcUnixUdpPollingTask::handleReadError: "
				<< strerror(errno) << std::endl;
	}
	}
}
