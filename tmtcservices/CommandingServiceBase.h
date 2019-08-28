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

/**
 * \brief This class is the basis for all PUS Services, which have to relay Telecommands to software bus.
 *
 * It manages Telecommand reception and the generation of Verification Reports like PUSServiceBase.
 * Every class that inherits from this abstract class has to implement four adaption points:
 *   - isValidSubservice
 *   - getMessageQueueAndObject
 *   - prepareCommand
 *   - handleReply
 * \ingroup pus_services
 */
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

	/**
	 * Class constructor. Initializes two important MessageQueues:
	 * commandQueue for command reception and requestQueue for device reception
	 * @param setObjectId
	 * @param apid
	 * @param service
	 * @param numberOfParallelCommands
	 * @param commandTimeout_seconds
	 * @param setPacketSource
	 * @param setPacketDestination
	 * @param queueDepth
	 */
	CommandingServiceBase(object_id_t setObjectId, uint16_t apid,
			uint8_t service, uint8_t numberOfParallelCommands,
			uint16_t commandTimeout_seconds, object_id_t setPacketSource,
			object_id_t setPacketDestination, size_t queueDepth = 20);
	virtual ~CommandingServiceBase();

	/***
	 * This is the periodic called function
	 * Handle request queue for external commands.
	 * Handle command Queue for internal commands.
	 * @param opCode is unused here at the moment
	 * @return RETURN_OK
	 */
	virtual ReturnValue_t performOperation(uint8_t opCode);

	virtual uint16_t getIdentifier();

	/**
	 * Returns the requestQueue MessageQueueId_t
	 *
	 * The requestQueue is the queue for external commands (TC)
	 *
	 * @return requestQueue messageQueueId_t
	 */
	virtual MessageQueueId_t getRequestQueue();

	/**
	 * Returns the commandQueue MessageQueueId_t
	 *
	 * Remember the CommandQueue is the queue for internal communication
	 * @return commandQueue messageQueueId_t
	 */
	virtual MessageQueueId_t getCommandQueue();

	virtual ReturnValue_t initialize();

	/**
	 * Implementation of ExecutableObjectIF function
	 *
	 * Used to setup the reference of the task, that executes this component
	 * @param task_ Pointer to the taskIF of this task
	 */
	virtual void setTaskIF(PeriodicTaskIF* task_){
		executingTask = task_;
	};

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
		uint32_t state;
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

	/**
	 * Pointer to the task which executes this component, is invalid before setTaskIF was called.
	 */
	PeriodicTaskIF* executingTask;

	/**
	 * Send TM data from pointer to data. If a header is supplied it is added before data
	 * @param subservice Number of subservice
	 * @param data Pointer to the data in the Packet
	 * @param dataLen Lenght of data in the Packet
	 * @param headerData HeaderData will be placed before data
	 * @param headerSize Size of HeaderData
	 */
	void sendTmPacket(uint8_t subservice, const uint8_t *data, uint32_t dataLen,
			const uint8_t* headerData = NULL, uint32_t headerSize = 0);

	/**
	 * To send TM packets of objects that still need to be serialized and consist of an object ID with appended data
	 * @param subservice Number of subservice
	 * @param objectId ObjectId is placed before data
	 * @param data Data to append to the packet
	 * @param dataLen Length of Data
	 */
	void sendTmPacket(uint8_t subservice, object_id_t objectId,
			const uint8_t *data, uint32_t dataLen);

	/**
	 * To send packets has data which is in form of a SerializeIF or Adapters implementing it
	 * @param subservice Number of subservice
	 * @param content This is a pointer to the serialized packet
	 * @param header Serialize IF header which will be placed before content
	 */
	void sendTmPacket(uint8_t subservice, SerializeIF* content,
			SerializeIF* header = NULL);
	virtual ReturnValue_t isValidSubservice(uint8_t subservice) = 0;

	virtual ReturnValue_t prepareCommand(CommandMessage *message,
			uint8_t subservice, const uint8_t *tcData, uint32_t tcDataLen,
			uint32_t *state, object_id_t objectId) = 0;

	virtual ReturnValue_t handleReply(const CommandMessage *reply,
			Command_t previousCommand, uint32_t *state,
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
	/**
	 * This method handles internal execution of a command,
	 * once it has been started by @sa{startExecution()} in the Request Queue handler.
	 * It handles replies generated by the devices and relayed by the specific service implementation.
	 * This means that it determines further course of action depending on the return values specified
	 * in the service implementation.
	 * This includes the generation of TC verification messages:
	 *   - TM[1,5] Step Successs
	 *   - TM[1,6] Step Failure
	 *   - TM[1,7] Completion Success
	 *   - TM[1,8] Completion Failure
	 */
	void handleCommandQueue();

	/**
	 * Sequence of request queue handling:
	 * isValidSubservice -> getMessageQueueAndObject -> startExecution
	 * Generates Start Success Reports TM[1,3] in subfunction @sa{startExecution()}
	 * or Start Failure Report TM[1,4] by using the TC Verification Service
	 */
	void handleRequestQueue();

	void rejectPacket(uint8_t reportId, TcPacketStored* packet,
			ReturnValue_t errorCode);

	void acceptPacket(uint8_t reportId, TcPacketStored* packet);

	void startExecution(TcPacketStored *storedPacket,
			typename FixedMap<MessageQueueId_t, CommandInfo>::Iterator *iter);

	void checkTimeout();
};

#endif /* COMMANDINGSERVICEBASE_H_ */
