#include "TcWinUdpPollingTask.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <winsock2.h>
#include <windows.h>

TcWinUdpPollingTask::TcWinUdpPollingTask(object_id_t objectId,
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

TcWinUdpPollingTask::~TcWinUdpPollingTask() {}

ReturnValue_t TcWinUdpPollingTask::performOperation(uint8_t opCode) {
	// Poll for new UDP datagrams in permanent loop.
	while(true) {
		//! Sender Address is cached here.
		struct sockaddr_in senderAddress;
		int senderAddressSize = sizeof(senderAddress);
		ssize_t bytesReceived = recvfrom(serverUdpSocket,
				reinterpret_cast<char*>(receptionBuffer.data()), frameSize,
				receptionFlags, reinterpret_cast<sockaddr*>(&senderAddress),
				&senderAddressSize);
		if(bytesReceived == SOCKET_ERROR) {
			// handle error
			sif::error << "TcWinUdpPollingTask::performOperation: Reception"
					" error." << std::endl;
			handleReadError();
			continue;
		}
		//sif::debug << "TcWinUdpPollingTask::performOperation: " << bytesReceived
		//		<< " bytes received" << std::endl;

		ReturnValue_t result = handleSuccessfullTcRead(bytesReceived);
		if(result != HasReturnvaluesIF::RETURN_FAILED) {

		}
		tmtcBridge->registerCommConnect();
		tmtcBridge->checkAndSetClientAddress(senderAddress);
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t TcWinUdpPollingTask::handleSuccessfullTcRead(size_t bytesRead) {
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

ReturnValue_t TcWinUdpPollingTask::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == nullptr) {
		sif::error << "TcSerialPollingTask::initialize: TC Store uninitialized!"
				<< std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	tmtcBridge = objectManager->get<TmTcWinUdpBridge>(tmtcBridgeId);
	if(tmtcBridge == nullptr) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Invalid"
				" TMTC bridge object!" << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	serverUdpSocket = tmtcBridge->serverSocket;
	//sif::info << "TcWinUdpPollingTask::initialize: Server UDP socket "
	//        << serverUdpSocket << std::endl;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcWinUdpPollingTask::initializeAfterTaskCreation() {
	// Initialize the destination after task creation. This ensures
	// that the destination has already been set in the TMTC bridge.
	targetTcDestination = tmtcBridge->getRequestQueue();
	return HasReturnvaluesIF::RETURN_OK;
}

void TcWinUdpPollingTask::setTimeout(double timeoutSeconds) {
	DWORD timeoutMs = timeoutSeconds * 1000.0;
	int result = setsockopt(serverUdpSocket, SOL_SOCKET, SO_RCVTIMEO,
			reinterpret_cast<const char*>(&timeoutMs), sizeof(DWORD));
	if(result == -1) {
		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Setting "
				"receive timeout failed with " << strerror(errno) << std::endl;
	}
}

void TcWinUdpPollingTask::handleReadError() {
    int error = WSAGetLastError();
    switch(error) {
    case(WSANOTINITIALISED): {
        sif::info << "TmTcWinUdpBridge::handleReadError: WSANOTINITIALISED: "
                << "WSAStartup(...) call " << "necessary" << std::endl;
        break;
    }
    case(WSAEFAULT): {
        sif::info << "TmTcWinUdpBridge::handleReadError: WSADEFAULT: "
                << "Bad address " << std::endl;
        break;
    }
    default: {
        sif::info << "TmTcWinUdpBridge::handleReadError: Error code: "
                << error << std::endl;
        break;
    }
    }
    // to prevent spam.
    Sleep(1000);
}
