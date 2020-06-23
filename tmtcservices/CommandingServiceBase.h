#ifndef FRAMEWORK_TMTCSERVICES_COMMANDINGSERVICEBASE_H_
#define FRAMEWORK_TMTCSERVICES_COMMANDINGSERVICEBASE_H_

#include <framework/objectmanager/SystemObject.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>

#include <framework/tmtcservices/VerificationReporter.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/container/FixedMap.h>
#include <framework/container/FIFO.h>
#include <framework/serialize/SerializeIF.h>

class TcPacketStored;

/**
 * @brief 	This class is the basis for all PUS Services, which have to
 * 			relay Telecommands to software bus.
 *
 * It manages Telecommand reception and the generation of Verification Reports
 * similar to PusServiceBase. This class is used if a telecommand can't be
 * handled immediately and must be relayed to the internal software bus.
 *   - isValidSubservice
 *   - getMessageQueueAndObject
 *   - prepareCommand
 *   - handleReply
 * @author gaisser
 * @ingroup pus_services
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
			uint16_t commandTimeoutSeconds, object_id_t setPacketSource,
			object_id_t setPacketDestination, size_t queueDepth = 20);
	virtual ~CommandingServiceBase();

	/***
	 * This is the periodically called function.
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
	virtual void setTaskIF(PeriodicTaskIF* task_);

protected:
	/**
	 * Check the target subservice
	 * @param subservice[in]
	 * @return -@c RETURN_OK on success
     *         -@c INVALID_SUBSERVICE if service is not known
	 */
	virtual ReturnValue_t isValidSubservice(uint8_t subservice) = 0;

	/**
	 * Once a TC Request is valid, the existence of the destination and its
	 * target interface is checked and retrieved. The target message queue ID
	 * can then be acquired by using the target interface.
	 * @param subservice
	 * @param tcData Application Data of TC Packet
	 * @param tcDataLen
	 * @param id MessageQueue ID is stored here
	 * @param objectId Object ID is extracted and stored here
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED
	 *         - @c CSB or implementation specific return codes
	 */
	virtual ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, size_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId) = 0;

	/**
	 * After the Message Queue and Object ID are determined, the command is
	 * prepared by using an implementation specific CommandMessage type
	 * which is sent to the target object. It contains all necessary information
	 * for the device to execute telecommands.
	 * @param message [out] message which can be set and is sent to the object
	 * @param subservice Subservice of the current communication
	 * @param tcData Application data of command
	 * @param tcDataLen Application data length
	 * @param state [out/in] Setable state of the communication.
	 * communication
	 * @param objectId Target object ID
	 * @return
	 */
	virtual ReturnValue_t prepareCommand(CommandMessage* message,
			uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
			uint32_t *state, object_id_t objectId) = 0;

	/**
	 * This function is implemented by child services to specify how replies
	 * to a command from another software component are handled.
	 * @param reply
	 * This is the reply which can be accessed via the command message
	 * interface. The internal message pointer can be passed to different
	 * command message implementations (see CommandMessageIF)
	 * @param previousCommand
	 * Command_t of related command
	 * @param state [out/in]
	 * Additional parameter which can be used to pass state information.
	 * State of the communication
	 * @param optionalNextCommand [out]
	 * An optional next command which can be set in this function
	 * @param objectId Source object ID
	 * @param isStep Flag value to mark steps of command execution
	 * @return
	 * - @c RETURN_OK, @c EXECUTION_COMPLETE or @c NO_STEP_MESSAGE to
	 *   generate TC verification success
	 * - @c INVALID_REPLY calls handleUnrequestedReply
	 * - Anything else triggers a TC verification failure. If RETURN_FAILED
	 *   is returned and the command ID is CommandMessage::REPLY_REJECTED,
	 *   a failure verification message with the reason as the error parameter
	 *   and the initial command as failure parameter 1.
	 */
	virtual ReturnValue_t handleReply(const CommandMessage* reply,
			Command_t previousCommand, uint32_t *state,
			CommandMessage* optionalNextCommand, object_id_t objectId,
			bool *isStep) = 0;

	/**
	 * This function can be overidden to handle unrequested reply,
	 * when the reply sender ID is unknown or is not found is the command map.
	 * The default implementation will clear the command message and all
	 * its contents.
	 * @param reply
	 * Reply which is non-const so the default implementation can clear the
	 * message.
	 */
	virtual void handleUnrequestedReply(CommandMessage* reply);

	virtual void doPeriodicOperation();


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

	using CommandMapIter = FixedMap<MessageQueueId_t,
	        CommandingServiceBase::CommandInfo>::Iterator;

	const uint16_t apid;

	const uint8_t service;

	const uint16_t timeoutSeconds;

	uint8_t tmPacketCounter = 0;

	StorageManagerIF *IPCStore = nullptr;

	StorageManagerIF *TCStore = nullptr;

	MessageQueueIF* commandQueue = nullptr;

	MessageQueueIF* requestQueue = nullptr;

	VerificationReporter verificationReporter;

	FixedMap<MessageQueueId_t, CommandInfo> commandMap;

	/* May be set be children to return a more precise failure condition. */
	uint32_t failureParameter1 = 0;
	uint32_t failureParameter2 = 0;

	object_id_t packetSource;

	object_id_t packetDestination;

	/**
	 * Pointer to the task which executes this component,
	 * is invalid before setTaskIF was called.
	 */
	PeriodicTaskIF* executingTask = nullptr;

	/**
	 * @brief   Send TM data from pointer to data.
	 *          If a header is supplied it is added before data
	 * @param subservice Number of subservice
	 * @param data Pointer to the data in the Packet
	 * @param dataLen Lenght of data in the Packet
	 * @param headerData HeaderData will be placed before data
	 * @param headerSize Size of HeaderData
	 */
	ReturnValue_t sendTmPacket(uint8_t subservice, const uint8_t *data,
	        size_t dataLen, const uint8_t* headerData = nullptr,
	        size_t headerSize = 0);

	/**
	 * @brief   To send TM packets of objects that still need to be serialized
	 *          and consist of an object ID with appended data.
	 * @param subservice Number of subservice
	 * @param objectId ObjectId is placed before data
	 * @param data Data to append to the packet
	 * @param dataLen Length of Data
	 */
	ReturnValue_t sendTmPacket(uint8_t subservice, object_id_t objectId,
			const uint8_t *data, size_t dataLen);

	/**
	 * @brief   To send packets which are contained inside a class implementing
	 *          SerializeIF.
	 * @param subservice Number of subservice
	 * @param content This is a pointer to the serialized packet
	 * @param header Serialize IF header which will be placed before content
	 */
	ReturnValue_t sendTmPacket(uint8_t subservice, SerializeIF* content,
			SerializeIF* header = nullptr);

	void checkAndExecuteFifo(CommandMapIter iter);

private:

	/**
	 * This method handles internal execution of a command,
	 * once it has been started by @sa{startExecution()} in the request
	 * queue handler.
	 * It handles replies generated by the devices and relayed by the specific
	 * service implementation. This means that it determines further course of
	 * action depending on the return values specified in the service
	 * implementation.
	 * This includes the generation of TC verification messages. Note that
	 * the static framework object ID @c VerificationReporter::messageReceiver
	 * needs to be set.
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

	void startExecution(TcPacketStored *storedPacket, CommandMapIter iter);

	void handleCommandMessage(CommandMessage* reply);
	void handleReplyHandlerResult(ReturnValue_t result, CommandMapIter iter,
			CommandMessage* nextCommand, CommandMessage* reply, bool& isStep);

	void checkTimeout();
};

#endif /* COMMANDINGSERVICEBASE_H_ */
