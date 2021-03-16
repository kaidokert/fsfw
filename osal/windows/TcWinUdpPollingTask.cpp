#include "TcWinUdpPollingTask.h"
#include "tcpipHelpers.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <winsock2.h>

//! Debugging preprocessor define.
#define FSFW_UDP_RCV_WIRETAPPING_ENABLED    0

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

	/* Set up reception buffer with specified frame size.
	For now, it is assumed that only one frame is held in the buffer! */
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
    /* Sender Address is cached here. */
    struct sockaddr_in senderAddress;
    int senderAddressSize = sizeof(senderAddress);

	/* Poll for new UDP datagrams in permanent loop. */
	while(true) {
		int bytesReceived = recvfrom(
		        serverUdpSocket,
		        reinterpret_cast<char*>(receptionBuffer.data()),
		        frameSize,
				receptionFlags,
				reinterpret_cast<sockaddr*>(&senderAddress),
				&senderAddressSize
		);
		if(bytesReceived == SOCKET_ERROR) {
			/* Handle error */
#if FSFW_CPP_OSTREAM_ENABLED == 1
			sif::error << "TcWinUdpPollingTask::performOperation: Reception error." << std::endl;
#endif
			tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::RECVFROM_CALL, 1000);
			continue;
		}
#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1
		sif::debug << "TcWinUdpPollingTask::performOperation: " << bytesReceived <<
		        " bytes received" << std::endl;
#endif

		ReturnValue_t result = handleSuccessfullTcRead(bytesReceived);
		if(result != HasReturnvaluesIF::RETURN_FAILED) {

		}
		tmtcBridge->checkAndSetClientAddress(senderAddress);
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t TcWinUdpPollingTask::handleSuccessfullTcRead(size_t bytesRead) {
	store_address_t storeId;

#if FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1
    arrayprinter::print(receptionBuffer.data(), bytesRead);
#endif

	ReturnValue_t result = tcStore->addData(&storeId, receptionBuffer.data(), bytesRead);
	if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning<< "TcWinUdpPollingTask::transferPusToSoftwareBus: Data storage failed." <<
		        std::endl;
		sif::warning << "Packet size: " << bytesRead << std::endl;
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	TmTcMessage message(storeId);

	result  = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
	if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "TcWinUdpPollingTask::handleSuccessfullTcRead: "
		        " Sending message to queue failed" << std::endl;
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
		tcStore->deleteData(storeId);
	}
	return result;
}

ReturnValue_t TcWinUdpPollingTask::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TcWinUdpPollingTask::initialize: TC store uninitialized!" << std::endl;
#endif
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	tmtcBridge = objectManager->get<TmTcWinUdpBridge>(tmtcBridgeId);
	if(tmtcBridge == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TcWinUdpPollingTask::initialize: Invalid TMTC bridge object!" <<
		        std::endl;
#endif
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcWinUdpPollingTask::initializeAfterTaskCreation() {
	/* Initialize the destination after task creation. This ensures
	that the destination has already been set in the TMTC bridge. */
	targetTcDestination = tmtcBridge->getRequestQueue();
	/* The server socket is set up in the bridge intialization. Calling this function here
	ensures that it is set up properly in any case*/
    serverUdpSocket = tmtcBridge->serverSocket;
	return HasReturnvaluesIF::RETURN_OK;
}

void TcWinUdpPollingTask::setTimeout(double timeoutSeconds) {
	DWORD timeoutMs = timeoutSeconds * 1000.0;
	int result = setsockopt(serverUdpSocket, SOL_SOCKET, SO_RCVTIMEO,
			reinterpret_cast<const char*>(&timeoutMs), sizeof(DWORD));
	if(result == -1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TcWinUdpPollingTask::TcSocketPollingTask: Setting "
				"receive timeout failed with " << strerror(errno) << std::endl;
#endif
	}
}
