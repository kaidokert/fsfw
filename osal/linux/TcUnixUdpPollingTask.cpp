//#include "TcUnixUdpPollingTask.h"
//#include "tcpipHelpers.h"
//
//#include "../../globalfunctions/arrayprinter.h"
//
//#define FSFW_UDP_RCV_WIRETAPPING_ENABLED    0
//
//TcUnixUdpPollingTask::TcUnixUdpPollingTask(object_id_t objectId,
//		object_id_t tmtcUnixUdpBridge, size_t frameSize,
//		double timeoutSeconds): SystemObject(objectId),
//		tmtcBridgeId(tmtcUnixUdpBridge) {
//
//	if(frameSize > 0) {
//		this->frameSize = frameSize;
//	}
//	else {
//		this->frameSize = DEFAULT_MAX_FRAME_SIZE;
//	}
//
//	/* Set up reception buffer with specified frame size.
//	For now, it is assumed that only one frame is held in the buffer! */
//	receptionBuffer.reserve(this->frameSize);
//	receptionBuffer.resize(this->frameSize);
//
//	if(timeoutSeconds == -1) {
//		receptionTimeout = DEFAULT_TIMEOUT;
//	}
//	else {
//		receptionTimeout = timevalOperations::toTimeval(timeoutSeconds);
//	}
//}
//
//TcUnixUdpPollingTask::~TcUnixUdpPollingTask() {}
//
//ReturnValue_t TcUnixUdpPollingTask::performOperation(uint8_t opCode) {
//    /* Sender Address is cached here. */
//    struct sockaddr_in senderAddress;
//    socklen_t senderAddressSize = sizeof(senderAddress);
//
//	/* Poll for new UDP datagrams in permanent loop. */
//	while(true) {
//		ssize_t bytesReceived = recvfrom(
//		        serverUdpSocket,
//				receptionBuffer.data(),
//				frameSize,
//				receptionFlags,
//				reinterpret_cast<sockaddr*>(&senderAddress),
//				&senderAddressSize
//		);
//		if(bytesReceived < 0) {
//			/* Handle error */
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//			sif::error << "TcSocketPollingTask::performOperation: Reception error." << std::endl;
//#endif
//			tcpip::handleError(tcpip::Protocol::UDP, tcpip::ErrorSources::RECVFROM_CALL, 500);
//			continue;
//		}
//#if FSFW_CPP_OSTREAM_ENABLED == 1 && FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1
//		sif::debug << "TcSocketPollingTask::performOperation: " << bytesReceived
//		        << " bytes received" << std::endl;
//#endif
//
//		ReturnValue_t result = handleSuccessfullTcRead(bytesReceived);
//		if(result != HasReturnvaluesIF::RETURN_FAILED) {
//
//		}
//		tmtcBridge->checkAndSetClientAddress(senderAddress);
//	}
//	return HasReturnvaluesIF::RETURN_OK;
//}
//
//
//ReturnValue_t TcUnixUdpPollingTask::handleSuccessfullTcRead(size_t bytesRead) {
//	store_address_t storeId;
//
//#if FSFW_UDP_RCV_WIRETAPPING_ENABLED == 1
//    arrayprinter::print(receptionBuffer.data(), bytesRead);
//#endif
//
//    ReturnValue_t result = tcStore->addData(&storeId, receptionBuffer.data(), bytesRead);
//	if (result != HasReturnvaluesIF::RETURN_OK) {
//#if FSFW_VERBOSE_LEVEL >= 1
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//		sif::error << "TcUnixUdpPollingTask::handleSuccessfullTcRead: Data "
//				"storage failed" << std::endl;
//		sif::error << "Packet size: " << bytesRead << std::endl;
//#else
//#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
//#endif /* FSFW_VERBOSE_LEVEL >= 1 */
//		return HasReturnvaluesIF::RETURN_FAILED;
//	}
//
//	TmTcMessage message(storeId);
//
//	result  = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
//	if (result != HasReturnvaluesIF::RETURN_OK) {
//#if FSFW_VERBOSE_LEVEL >= 1
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//		sif::error << "TcUnixUdpPollingTask::handleSuccessfullTcRead: Sending message to queue "
//		        "failed" << std::endl;
//#else
//#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
//#endif /* FSFW_VERBOSE_LEVEL >= 1 */
//		tcStore->deleteData(storeId);
//	}
//	return result;
//}
//
//ReturnValue_t TcUnixUdpPollingTask::initialize() {
//	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
//	if (tcStore == nullptr) {
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//		sif::error << "TcSerialPollingTask::initialize: TC Store uninitialized!"
//				<< std::endl;
//#endif
//		return ObjectManagerIF::CHILD_INIT_FAILED;
//	}
//
//	tmtcBridge = objectManager->get<TmTcUnixUdpBridge>(tmtcBridgeId);
//	if(tmtcBridge == nullptr) {
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Invalid"
//				" TMTC bridge object!" << std::endl;
//#endif
//		return ObjectManagerIF::CHILD_INIT_FAILED;
//	}
//
//	return HasReturnvaluesIF::RETURN_OK;
//}
//
//ReturnValue_t TcUnixUdpPollingTask::initializeAfterTaskCreation() {
//    /* Initialize the destination after task creation. This ensures
//    that the destination has already been set in the TMTC bridge. */
//	targetTcDestination = tmtcBridge->getRequestQueue();
//    /* The server socket is set up in the bridge intialization. Calling this function here
//    ensures that it is set up properly in any case*/
//    serverUdpSocket = tmtcBridge->serverSocket;
//	return HasReturnvaluesIF::RETURN_OK;
//}
//
//void TcUnixUdpPollingTask::setTimeout(double timeoutSeconds) {
//	timeval tval;
//	tval = timevalOperations::toTimeval(timeoutSeconds);
//	int result = setsockopt(serverUdpSocket, SOL_SOCKET, SO_RCVTIMEO,
//			&tval, sizeof(receptionTimeout));
//	if(result == -1) {
//#if FSFW_CPP_OSTREAM_ENABLED == 1
//		sif::error << "TcSocketPollingTask::TcSocketPollingTask: Setting "
//				"receive timeout failed with " << strerror(errno) << std::endl;
//#endif
//	}
//}
