/*
 * CommandingServiceBase.cpp
 *
 *  Created on: 28.08.2019
 *      Author: gaisser
 */

#include <framework/tmtcservices/CommandingServiceBase.h>

CommandingServiceBase::CommandingServiceBase(object_id_t setObjectId,
		uint16_t apid, uint8_t service, uint8_t numberOfParallelCommands,
		uint16_t commandTimeout_seconds, object_id_t setPacketSource,
		object_id_t setPacketDestination, size_t queueDepth) :
		SystemObject(setObjectId), apid(apid), service(service),
		timeout_seconds(commandTimeout_seconds), tmPacketCounter(0), IPCStore(NULL),
		TCStore(NULL), commandQueue(NULL), requestQueue(NULL), commandMap(numberOfParallelCommands),
		failureParameter1(0), failureParameter2(0), packetSource(setPacketSource),
		packetDestination(setPacketDestination),executingTask(NULL) {
	commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
	requestQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
}


CommandingServiceBase::~CommandingServiceBase() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
	QueueFactory::instance()->deleteMessageQueue(requestQueue);
}


ReturnValue_t CommandingServiceBase::performOperation(uint8_t opCode) {
	handleCommandQueue();
	handleRequestQueue();
	checkTimeout();
	doPeriodicOperation();
	return RETURN_OK;
}


uint16_t CommandingServiceBase::getIdentifier() {
	return service;
}


MessageQueueId_t CommandingServiceBase::getRequestQueue() {
	return requestQueue->getId();
}


ReturnValue_t CommandingServiceBase::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	AcceptsTelemetryIF* packetForwarding =
			objectManager->get<AcceptsTelemetryIF>(packetDestination);
	PUSDistributorIF* distributor = objectManager->get<PUSDistributorIF>(
			packetSource);
	if ((packetForwarding == NULL) && (distributor == NULL)) {
		return RETURN_FAILED;
	}

	distributor->registerService(this);
	requestQueue->setDefaultDestination(
			packetForwarding->getReportReceptionQueue());

	IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	TCStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);

	if ((IPCStore == NULL) || (TCStore == NULL)) {
		return RETURN_FAILED;
	}

	return RETURN_OK;

}

void CommandingServiceBase::handleCommandQueue() {
	CommandMessage reply, nextCommand;
	ReturnValue_t result, sendResult = RETURN_OK;
	bool isStep = false;
	for (result = commandQueue->receiveMessage(&reply); result == RETURN_OK;
			result = commandQueue->receiveMessage(&reply)) {
		isStep = false;
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase::CommandInfo>::Iterator iter;
		if (reply.getSender() == MessageQueueIF::NO_QUEUE) {
			handleUnrequestedReply(&reply);
			continue;
		}
		if ((iter = commandMap.find(reply.getSender())) == commandMap.end()) {
			handleUnrequestedReply(&reply);
			continue;
		}
		nextCommand.setCommand(CommandMessage::CMD_NONE);
		result = handleReply(&reply, iter->command, &iter->state, &nextCommand,
				iter->objectId, &isStep);
		switch (result) {
		case EXECUTION_COMPLETE:
		case RETURN_OK:
		case NO_STEP_MESSAGE:
			iter->command = nextCommand.getCommand();
			if (nextCommand.getCommand() != CommandMessage::CMD_NONE) {
				sendResult = commandQueue->sendMessage(reply.getSender(),
						&nextCommand);
			}
			if (sendResult == RETURN_OK) {
				if (isStep) {
					if (result != NO_STEP_MESSAGE) {
						verificationReporter.sendSuccessReport(
								TC_VERIFY::PROGRESS_SUCCESS,
								iter->tcInfo.ackFlags, iter->tcInfo.tcPacketId,
								iter->tcInfo.tcSequenceControl, ++iter->step);
					}
				} else {
					verificationReporter.sendSuccessReport(
							TC_VERIFY::COMPLETION_SUCCESS,
							iter->tcInfo.ackFlags, iter->tcInfo.tcPacketId,
							iter->tcInfo.tcSequenceControl, 0);
					checkAndExecuteFifo(&iter);
				}
			} else {
				if (isStep) {
					nextCommand.clearCommandMessage();
					verificationReporter.sendFailureReport(
							TC_VERIFY::PROGRESS_FAILURE, iter->tcInfo.ackFlags,
							iter->tcInfo.tcPacketId,
							iter->tcInfo.tcSequenceControl, sendResult,
							++iter->step, failureParameter1, failureParameter2);
				} else {
					nextCommand.clearCommandMessage();
					verificationReporter.sendFailureReport(
							TC_VERIFY::COMPLETION_FAILURE,
							iter->tcInfo.ackFlags, iter->tcInfo.tcPacketId,
							iter->tcInfo.tcSequenceControl, sendResult, 0,
							failureParameter1, failureParameter2);
				}
				failureParameter1 = 0;
				failureParameter2 = 0;
				checkAndExecuteFifo(&iter);
			}
			break;
		case INVALID_REPLY:
			//might be just an unrequested reply at a bad moment
			handleUnrequestedReply(&reply);
			break;
		default:
			if (isStep) {
				verificationReporter.sendFailureReport(
						TC_VERIFY::PROGRESS_FAILURE, iter->tcInfo.ackFlags,
						iter->tcInfo.tcPacketId, iter->tcInfo.tcSequenceControl,
						result, ++iter->step, failureParameter1,
						failureParameter2);
			} else {
				verificationReporter.sendFailureReport(
						TC_VERIFY::COMPLETION_FAILURE, iter->tcInfo.ackFlags,
						iter->tcInfo.tcPacketId, iter->tcInfo.tcSequenceControl,
						result, 0, failureParameter1, failureParameter2);
			}
			failureParameter1 = 0;
			failureParameter2 = 0;
			checkAndExecuteFifo(&iter);
			break;
		}

	}
}


void CommandingServiceBase::handleRequestQueue() {
	TmTcMessage message;
	ReturnValue_t result;
	store_address_t address;
	TcPacketStored packet;
	MessageQueueId_t queue;
	object_id_t objectId;
	for (result = requestQueue->receiveMessage(&message); result == RETURN_OK;
			result = requestQueue->receiveMessage(&message)) {
		address = message.getStorageId();
		packet.setStoreAddress(address);

		if ((packet.getSubService() == 0)
				|| (isValidSubservice(packet.getSubService()) != RETURN_OK)) {
			rejectPacket(TC_VERIFY::START_FAILURE, &packet, INVALID_SUBSERVICE);
			continue;
		}
		result = getMessageQueueAndObject(packet.getSubService(),
				packet.getApplicationData(), packet.getApplicationDataSize(),
				&queue, &objectId);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			rejectPacket(TC_VERIFY::START_FAILURE, &packet, result);
			continue;
		}

		//Is a command already active for the target object?
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase::CommandInfo>::Iterator iter;
		iter = commandMap.find(queue);

		if (iter != commandMap.end()) {
			result = iter->fifo.insert(address);
			if (result != RETURN_OK) {
				rejectPacket(TC_VERIFY::START_FAILURE, &packet, OBJECT_BUSY);
			}
		} else {
			CommandInfo newInfo; //Info will be set by startExecution if neccessary
			newInfo.objectId = objectId;
			result = commandMap.insert(queue, newInfo, &iter);
			if (result != RETURN_OK) {
				rejectPacket(TC_VERIFY::START_FAILURE, &packet, BUSY);
			} else {
				startExecution(&packet, &iter);
			}
		}

	}
}


void CommandingServiceBase::sendTmPacket(uint8_t subservice,
		const uint8_t* data, uint32_t dataLen, const uint8_t* headerData,
		uint32_t headerSize) {
	TmPacketStored tmPacketStored(this->apid, this->service, subservice,
			this->tmPacketCounter, data, dataLen, headerData, headerSize);
	ReturnValue_t result = tmPacketStored.sendPacket(
			requestQueue->getDefaultDestination(), requestQueue->getId());
	if (result == HasReturnvaluesIF::RETURN_OK) {
		this->tmPacketCounter++;
	}
}


void CommandingServiceBase::sendTmPacket(uint8_t subservice,
		object_id_t objectId, const uint8_t *data, uint32_t dataLen) {
	uint8_t buffer[sizeof(object_id_t)];
	uint8_t* pBuffer = buffer;
	uint32_t size = 0;
	SerializeAdapter<object_id_t>::serialize(&objectId, &pBuffer, &size,
			sizeof(object_id_t), true);
	TmPacketStored tmPacketStored(this->apid, this->service, subservice,
			this->tmPacketCounter, data, dataLen, buffer, size);
	ReturnValue_t result = tmPacketStored.sendPacket(
			requestQueue->getDefaultDestination(), requestQueue->getId());
	if (result == HasReturnvaluesIF::RETURN_OK) {
		this->tmPacketCounter++;
	}

}


void CommandingServiceBase::sendTmPacket(uint8_t subservice,
		SerializeIF* content, SerializeIF* header) {
	TmPacketStored tmPacketStored(this->apid, this->service, subservice,
			this->tmPacketCounter, content, header);
	ReturnValue_t result = tmPacketStored.sendPacket(
			requestQueue->getDefaultDestination(), requestQueue->getId());
	if (result == HasReturnvaluesIF::RETURN_OK) {
		this->tmPacketCounter++;
	}
}


void CommandingServiceBase::startExecution(
		TcPacketStored *storedPacket,
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase::CommandInfo>::Iterator *iter) {
	ReturnValue_t result, sendResult = RETURN_OK;
	CommandMessage message;
	(*iter)->subservice = storedPacket->getSubService();
	result = prepareCommand(&message, (*iter)->subservice,
			storedPacket->getApplicationData(),
			storedPacket->getApplicationDataSize(), &(*iter)->state,
			(*iter)->objectId);

	switch (result) {
	case RETURN_OK:
		if (message.getCommand() != CommandMessage::CMD_NONE) {
			sendResult = commandQueue->sendMessage((*iter).value->first,
					&message);
		}
		if (sendResult == RETURN_OK) {
			Clock::getUptime(&(*iter)->uptimeOfStart);
			(*iter)->step = 0;
//			(*iter)->state = 0;
			(*iter)->subservice = storedPacket->getSubService();
			(*iter)->command = message.getCommand();
			(*iter)->tcInfo.ackFlags = storedPacket->getAcknowledgeFlags();
			(*iter)->tcInfo.tcPacketId = storedPacket->getPacketId();
			(*iter)->tcInfo.tcSequenceControl =
					storedPacket->getPacketSequenceControl();
			acceptPacket(TC_VERIFY::START_SUCCESS, storedPacket);
		} else {
			message.clearCommandMessage();
			rejectPacket(TC_VERIFY::START_FAILURE, storedPacket, sendResult);
			checkAndExecuteFifo(iter);
		}
		break;
	case EXECUTION_COMPLETE:
		if (message.getCommand() != CommandMessage::CMD_NONE) {
			//Fire-and-forget command.
			sendResult = commandQueue->sendMessage((*iter).value->first,
					&message);
		}
		if (sendResult == RETURN_OK) {
			verificationReporter.sendSuccessReport(TC_VERIFY::START_SUCCESS,
					storedPacket);
			acceptPacket(TC_VERIFY::COMPLETION_SUCCESS, storedPacket);
			checkAndExecuteFifo(iter);
		} else {
			message.clearCommandMessage();
			rejectPacket(TC_VERIFY::START_FAILURE, storedPacket, sendResult);
			checkAndExecuteFifo(iter);
		}
		break;
	default:
		rejectPacket(TC_VERIFY::START_FAILURE, storedPacket, result);
		checkAndExecuteFifo(iter);
		break;
	}
}


void CommandingServiceBase::rejectPacket(uint8_t report_id,
		TcPacketStored* packet, ReturnValue_t error_code) {
	verificationReporter.sendFailureReport(report_id, packet, error_code);
	packet->deletePacket();
}


void CommandingServiceBase::acceptPacket(uint8_t reportId,
		TcPacketStored* packet) {
	verificationReporter.sendSuccessReport(reportId, packet);
	packet->deletePacket();
}


void CommandingServiceBase::checkAndExecuteFifo(
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase::CommandInfo>::Iterator *iter) {
	store_address_t address;
	if ((*iter)->fifo.retrieve(&address) != RETURN_OK) {
		commandMap.erase(iter);
	} else {
		TcPacketStored newPacket(address);
		startExecution(&newPacket, iter);
	}
}


void CommandingServiceBase::handleUnrequestedReply(
		CommandMessage* reply) {
	reply->clearCommandMessage();
}


inline void CommandingServiceBase::doPeriodicOperation() {
}

MessageQueueId_t CommandingServiceBase::getCommandQueue() {
	return commandQueue->getId();
}

void CommandingServiceBase::checkTimeout() {
	uint32_t uptime;
	Clock::getUptime(&uptime);
	typename FixedMap<MessageQueueId_t,
			CommandingServiceBase::CommandInfo>::Iterator iter;
	for (iter = commandMap.begin(); iter != commandMap.end(); ++iter) {
		if ((iter->uptimeOfStart + (timeout_seconds * 1000)) < uptime) {
			verificationReporter.sendFailureReport(
					TC_VERIFY::COMPLETION_FAILURE, iter->tcInfo.ackFlags,
					iter->tcInfo.tcPacketId, iter->tcInfo.tcSequenceControl,
					TIMEOUT);
			checkAndExecuteFifo(&iter);
		}
	}
}



