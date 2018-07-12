#ifndef COMMANDINGSERVICEBASE_H_
#define COMMANDINGSERVICEBASE_H_

#include <framework/container/FixedMap.h>
#include <framework/container/FIFO.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/tcdistribution/PUSDistributorIF.h>
#include <framework/tmtcpacket/pus/TcPacketStored.h>
#include <framework/tmtcpacket/pus/TmPacketStored.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tmtcservices/TmTcMessage.h>
#include <framework/tmtcservices/VerificationReporter.h>
#include <framework/internalError/InternalErrorReporterIF.h>
#include <framework/ipc/QueueFactory.h>
#include <framework/timemanager/Clock.h>

template<typename STATE_T>
class CommandingServiceBase: public SystemObject,
		public AcceptsTelecommandsIF,
		public ExecutableObjectIF,
		public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::COMMAND_SERVICE_BASE;
	static const ReturnValue_t EXECUTION_COMPLETE = MAKE_RETURN_CODE(1);
	static const ReturnValue_t NO_STEP_MESSAGE = MAKE_RETURN_CODE(2);
	static const ReturnValue_t OBJECT_BUSY = MAKE_RETURN_CODE(3);
	static const ReturnValue_t BUSY = MAKE_RETURN_CODE(4);
	static const ReturnValue_t INVALID_TC = MAKE_RETURN_CODE(5);
	static const ReturnValue_t INVALID_OBJECT = MAKE_RETURN_CODE(6);
	static const ReturnValue_t INVALID_REPLY = MAKE_RETURN_CODE(7);

	CommandingServiceBase(object_id_t setObjectId, uint16_t apid,
			uint8_t service, uint8_t numberOfParallelCommands,
			uint16_t commandTimeout_seconds, object_id_t setPacketSource,
			object_id_t setPacketDestination, size_t queueDepth = 20);
	virtual ~CommandingServiceBase();

	virtual ReturnValue_t performOperation(uint8_t opCode);

	virtual uint16_t getIdentifier();

	virtual MessageQueueId_t getRequestQueue();

	virtual ReturnValue_t initialize();

protected:
	struct CommandInfo {
		struct tcInfo {
			uint8_t ackFlags;
			uint16_t tcPacketId;
			uint16_t tcSequenceControl;
		} tcInfo;
		uint32_t uptimeOfStart;
		uint8_t step;
		uint8_t subservice;
		STATE_T state;
		Command_t command;
		object_id_t objectId;
		FIFO<store_address_t, 3> fifo;
	};

	const uint16_t apid;

	const uint8_t service;

	const uint16_t timeout_seconds;

	uint8_t tmPacketCounter;

	StorageManagerIF *IPCStore;

	StorageManagerIF *TCStore;

	MessageQueueIF* commandQueue;

	MessageQueueIF* requestQueue;

	VerificationReporter verificationReporter;

	FixedMap<MessageQueueId_t, CommandInfo> commandMap;

	uint32_t failureParameter1; //!< May be set be children to return a more precise failure condition.
	uint32_t failureParameter2; //!< May be set be children to return a more precise failure condition.

	object_id_t packetSource;

	object_id_t packetDestination;

	void sendTmPacket(uint8_t subservice, const uint8_t *data, uint32_t dataLen,
			const uint8_t* headerData = NULL, uint32_t headerSize = 0);
	void sendTmPacket(uint8_t subservice, object_id_t objectId,
			const uint8_t *data, uint32_t dataLen);

	void sendTmPacket(uint8_t subservice, SerializeIF* content,
			SerializeIF* header = NULL);
	virtual ReturnValue_t isValidSubservice(uint8_t subservice) = 0;

	virtual ReturnValue_t prepareCommand(CommandMessage *message,
			uint8_t subservice, const uint8_t *tcData, uint32_t tcDataLen,
			STATE_T *state, object_id_t objectId) = 0;

	virtual ReturnValue_t handleReply(const CommandMessage *reply,
			Command_t previousCommand, STATE_T *state,
			CommandMessage *optionalNextCommand, object_id_t objectId,
			bool *isStep) = 0;

	virtual ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, uint32_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId) = 0;

	virtual void handleUnrequestedReply(CommandMessage *reply);

	virtual void doPeriodicOperation();

	void checkAndExecuteFifo(
			typename FixedMap<MessageQueueId_t, CommandInfo>::Iterator *iter);

private:
	void handleCommandQueue();

	void handleRequestQueue();

	void rejectPacket(uint8_t reportId, TcPacketStored* packet,
			ReturnValue_t errorCode);

	void acceptPacket(uint8_t reportId, TcPacketStored* packet);

	void startExecution(TcPacketStored *storedPacket,
			typename FixedMap<MessageQueueId_t, CommandInfo>::Iterator *iter);

	void checkTimeout();
};

template<typename STATE_T>
CommandingServiceBase<STATE_T>::CommandingServiceBase(object_id_t setObjectId,
		uint16_t apid, uint8_t service, uint8_t numberOfParallelCommands,
		uint16_t commandTimeout_seconds, object_id_t setPacketSource,
		object_id_t setPacketDestination, size_t queueDepth) :
		SystemObject(setObjectId), apid(apid), service(service), timeout_seconds(
				commandTimeout_seconds), tmPacketCounter(0), IPCStore(NULL), TCStore(
		NULL), commandQueue(NULL), requestQueue(NULL), commandMap(
				numberOfParallelCommands), failureParameter1(0), failureParameter2(
				0), packetSource(setPacketSource), packetDestination(
				setPacketDestination) {
	commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
	requestQueue = QueueFactory::instance()->createMessageQueue(20); //TODO: Funny magic number.
}

template<typename STATE_T>
CommandingServiceBase<STATE_T>::~CommandingServiceBase() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
	QueueFactory::instance()->deleteMessageQueue(requestQueue);
}

template<typename STATE_T>
ReturnValue_t CommandingServiceBase<STATE_T>::performOperation(uint8_t opCode) {
	handleCommandQueue();
	handleRequestQueue();
	checkTimeout();
	doPeriodicOperation();
	return RETURN_OK;
}

template<typename STATE_T>
uint16_t CommandingServiceBase<STATE_T>::getIdentifier() {
	return service;
}

template<typename STATE_T>
MessageQueueId_t CommandingServiceBase<STATE_T>::getRequestQueue() {
	return requestQueue->getId();
}

template<typename STATE_T>
ReturnValue_t CommandingServiceBase<STATE_T>::initialize() {
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

//Whole method works like this, but I don't like it. Leave it anyway.
template<typename STATE_T>
void CommandingServiceBase<STATE_T>::handleCommandQueue() {
	CommandMessage reply, nextCommand;
	ReturnValue_t result, sendResult = RETURN_OK;
	bool isStep = false;
	for (result = commandQueue->receiveMessage(&reply); result == RETURN_OK;
			result = commandQueue->receiveMessage(&reply)) {
		isStep = false;
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase<STATE_T>::CommandInfo>::Iterator iter;
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

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::handleRequestQueue() {
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

		//is a command already active for the target object?
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase<STATE_T>::CommandInfo>::Iterator iter;
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

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::sendTmPacket(uint8_t subservice,
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

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::sendTmPacket(uint8_t subservice,
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

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::sendTmPacket(uint8_t subservice,
		SerializeIF* content, SerializeIF* header) {
	TmPacketStored tmPacketStored(this->apid, this->service, subservice,
			this->tmPacketCounter, content, header);
	ReturnValue_t result = tmPacketStored.sendPacket(
			requestQueue->getDefaultDestination(), requestQueue->getId());
	if (result == HasReturnvaluesIF::RETURN_OK) {
		this->tmPacketCounter++;
	}
}

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::startExecution(
		TcPacketStored *storedPacket,
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase<STATE_T>::CommandInfo>::Iterator *iter) {
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

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::rejectPacket(uint8_t report_id,
		TcPacketStored* packet, ReturnValue_t error_code) {
	verificationReporter.sendFailureReport(report_id, packet, error_code);
	packet->deletePacket();
}

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::acceptPacket(uint8_t reportId,
		TcPacketStored* packet) {
	verificationReporter.sendSuccessReport(reportId, packet);
	packet->deletePacket();
}

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::checkAndExecuteFifo(
		typename FixedMap<MessageQueueId_t,
				CommandingServiceBase<STATE_T>::CommandInfo>::Iterator *iter) {
	store_address_t address;
	if ((*iter)->fifo.retrieve(&address) != RETURN_OK) {
		commandMap.erase(iter);
	} else {
		TcPacketStored newPacket(address);
		startExecution(&newPacket, iter);
	}
}

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::handleUnrequestedReply(
		CommandMessage* reply) {
	reply->clearCommandMessage();
}

template<typename STATE_T>
inline void CommandingServiceBase<STATE_T>::doPeriodicOperation() {
}

template<typename STATE_T>
void CommandingServiceBase<STATE_T>::checkTimeout() {
	uint32_t uptime;
	Clock::getUptime(&uptime);
	typename FixedMap<MessageQueueId_t,
			CommandingServiceBase<STATE_T>::CommandInfo>::Iterator iter;
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
#endif /* COMMANDINGSERVICEBASE_H_ */
