#include "DeviceHandlerBase.h"
#include "AcceptsDeviceResponsesIF.h"
#include "DeviceTmReportingWrapper.h"

#include "../objectmanager/ObjectManager.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../thermal/ThermalComponentIF.h"
#include "../datapool/DataSet.h"
#include "../datapool/PoolVariable.h"
#include "../globalfunctions/CRC.h"
#include "../subsystem/SubsystemBase.h"
#include "../ipc/QueueFactory.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

#include <iomanip>

object_id_t DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
object_id_t DeviceHandlerBase::rawDataReceiverId = objects::NO_OBJECT;
object_id_t DeviceHandlerBase::defaultFdirParentId = objects::NO_OBJECT;

DeviceHandlerBase::DeviceHandlerBase(object_id_t setObjectId,
		object_id_t deviceCommunication, CookieIF * comCookie,
		FailureIsolationBase* fdirInstance, size_t cmdQueueSize) :
		SystemObject(setObjectId), mode(MODE_OFF), submode(SUBMODE_NONE),
		wiretappingMode(OFF), storedRawData(StorageManagerIF::INVALID_ADDRESS),
		deviceCommunicationId(deviceCommunication), comCookie(comCookie),
		healthHelper(this,setObjectId), modeHelper(this), parameterHelper(this),
		actionHelper(this, nullptr), childTransitionFailure(RETURN_OK),
		fdirInstance(fdirInstance), hkSwitcher(this),
		defaultFDIRUsed(fdirInstance == nullptr), switchOffWasReported(false),
		childTransitionDelay(5000), transitionSourceMode(_MODE_POWER_DOWN),
		transitionSourceSubMode(SUBMODE_NONE) {
	commandQueue = QueueFactory::instance()->createMessageQueue(cmdQueueSize,
			MessageQueueMessage::MAX_MESSAGE_SIZE);
	insertInCommandMap(RAW_COMMAND_ID);
	cookieInfo.state = COOKIE_UNUSED;
	cookieInfo.pendingCommand = deviceCommandMap.end();
	if (comCookie == nullptr) {
		sif::error << "DeviceHandlerBase: ObjectID 0x" << std::hex
				<< std::setw(8) << std::setfill('0') << this->getObjectId()
				<< std::dec << ": Do not pass nullptr as a cookie, consider "
				<< std::setfill(' ') << "passing a dummy cookie instead!"
				<< std::endl;
	}
	if (this->fdirInstance == nullptr) {
		this->fdirInstance = new DeviceHandlerFailureIsolation(setObjectId,
				defaultFdirParentId);
	}
}

void DeviceHandlerBase::setThermalStateRequestPoolIds(
		uint32_t thermalStatePoolId, uint32_t thermalRequestPoolId) {
	this->deviceThermalRequestPoolId = thermalStatePoolId;
	this->deviceThermalRequestPoolId = thermalRequestPoolId;
}


DeviceHandlerBase::~DeviceHandlerBase() {
	delete comCookie;
	if (defaultFDIRUsed) {
		delete fdirInstance;
	}
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t DeviceHandlerBase::performOperation(uint8_t counter) {
	this->pstStep = counter;

	if (getComAction() == SEND_WRITE) {
		cookieInfo.state = COOKIE_UNUSED;
		readCommandQueue();
		doStateMachine();
		checkSwitchState();
		decrementDeviceReplyMap();
		fdirInstance->checkForFailures();
		hkSwitcher.performOperation();
		performOperationHook();
	}
	if (mode == MODE_OFF) {
		return RETURN_OK;
	}
	switch (getComAction()) {
	case SEND_WRITE:
		if ((cookieInfo.state == COOKIE_UNUSED)) {
			buildInternalCommand();
		}
		doSendWrite();
		break;
	case GET_WRITE:
		doGetWrite();
		break;
	case SEND_READ:
		doSendRead();
		break;
	case GET_READ:
		doGetRead();
		cookieInfo.state = COOKIE_UNUSED;
		break;
	default:
		break;
	}
	return RETURN_OK;
}

ReturnValue_t DeviceHandlerBase::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}

	communicationInterface = objectManager->get<DeviceCommunicationIF>(
			deviceCommunicationId);
	if (communicationInterface == nullptr) {
		sif::error << "DeviceHandlerBase::initialize: Communication interface "
				"invalid." << std::endl;
		sif::error << "Make sure it is set up properly and implements"
				" DeviceCommunicationIF" << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	result = communicationInterface->initializeInterface(comCookie);
	if (result != RETURN_OK) {
		return result;
	}

	IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (IPCStore == nullptr) {
		sif::error << "DeviceHandlerBase::initialize: IPC store not set up in "
				"factory." << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	if(rawDataReceiverId != objects::NO_OBJECT) {
		AcceptsDeviceResponsesIF *rawReceiver = objectManager->get<
				AcceptsDeviceResponsesIF>(rawDataReceiverId);

		if (rawReceiver == nullptr) {
			sif::error << "DeviceHandlerBase::initialize: Raw receiver object "
					"ID set but no valid object found." << std::endl;
			sif::error << "Make sure the raw receiver object is set up properly"
					" and implements AcceptsDeviceResponsesIF" << std::endl;
			return ObjectManagerIF::CHILD_INIT_FAILED;
		}
		defaultRawReceiver = rawReceiver->getDeviceQueue();
	}

	if(powerSwitcherId != objects::NO_OBJECT) {
		powerSwitcher = objectManager->get<PowerSwitchIF>(powerSwitcherId);
		if (powerSwitcher == nullptr) {
			sif::error << "DeviceHandlerBase::initialize: Power switcher "
					<< "object ID set but no valid object found." << std::endl;
			sif::error << "Make sure the raw receiver object is set up properly"
					<< " and implements PowerSwitchIF" << std::endl;
			return ObjectManagerIF::CHILD_INIT_FAILED;
		}
	}

	result = healthHelper.initialize();
	if (result != RETURN_OK) {
	    return result;
	}

	result = modeHelper.initialize();
	if (result != RETURN_OK) {
		return result;
	}
	result = actionHelper.initialize(commandQueue);
	if (result != RETURN_OK) {
		return result;
	}
	result = fdirInstance->initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = parameterHelper.initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = hkSwitcher.initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	fillCommandAndReplyMap();

	//Set temperature target state to NON_OP.
	DataSet mySet;
	db_int8_t thermalRequest(deviceThermalRequestPoolId, &mySet,
			PoolVariableIF::VAR_WRITE);
	mySet.read();
	thermalRequest = ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL;
	mySet.commit(PoolVariableIF::VALID);

	return RETURN_OK;

}

void DeviceHandlerBase::decrementDeviceReplyMap() {
	for (std::map<DeviceCommandId_t, DeviceReplyInfo>::iterator iter =
			deviceReplyMap.begin(); iter != deviceReplyMap.end(); iter++) {
		if (iter->second.delayCycles != 0) {
			iter->second.delayCycles--;
			if (iter->second.delayCycles == 0) {
				if (iter->second.periodic) {
					iter->second.delayCycles = iter->second.maxDelayCycles;
				}
				replyToReply(iter, TIMEOUT);
				missedReply(iter->first);
			}
		}
	}
}

void DeviceHandlerBase::readCommandQueue() {

	if (dontCheckQueue()) {
		return;
	}

	CommandMessage command;
	ReturnValue_t result = commandQueue->receiveMessage(&command);
	if (result != RETURN_OK) {
		return;
	}

	result = healthHelper.handleHealthCommand(&command);
	if (result == RETURN_OK) {
	    return;
	}

	result = modeHelper.handleModeCommand(&command);
	if (result == RETURN_OK) {
		return;
	}

	result = actionHelper.handleActionMessage(&command);
	if (result == RETURN_OK) {
		return;
	}

	result = parameterHelper.handleParameterMessage(&command);
	if (result == RETURN_OK) {
		return;
	}

//	result = hkManager.handleHousekeepingMessage(&command);
//	if (result == RETURN_OK) {
//		return;
//	}

	result = handleDeviceHandlerMessage(&command);
	if (result == RETURN_OK) {
		return;
	}

	result = letChildHandleMessage(&command);
	if (result == RETURN_OK) {
		return;
	}

	replyReturnvalueToCommand(CommandMessage::UNKNOWN_COMMAND);

}

void DeviceHandlerBase::doStateMachine() {
	switch (mode) {
	case _MODE_START_UP:
	case _MODE_SHUT_DOWN:
	case _MODE_TO_NORMAL:
	case _MODE_TO_ON:
	case _MODE_TO_RAW: {
		Mode_t currentMode = mode;
		callChildStatemachine();
		//Only do timeout if child did not change anything
		if (mode != currentMode) {
			break;
		}
		uint32_t currentUptime;
		Clock::getUptime(&currentUptime);
		if (currentUptime - timeoutStart >= childTransitionDelay) {
			triggerEvent(MODE_TRANSITION_FAILED, childTransitionFailure, 0);
			setMode(transitionSourceMode, transitionSourceSubMode);
			break;
		}
	}
		break;
	case _MODE_POWER_DOWN:
		commandSwitch(PowerSwitchIF::SWITCH_OFF);
		setMode(_MODE_WAIT_OFF);
		break;
	case _MODE_POWER_ON:
		commandSwitch(PowerSwitchIF::SWITCH_ON);
		setMode(_MODE_WAIT_ON);
		break;
	case _MODE_WAIT_ON: {
		uint32_t currentUptime;
		Clock::getUptime(&currentUptime);
		if (powerSwitcher != nullptr and currentUptime - timeoutStart >=
				powerSwitcher->getSwitchDelayMs()) {
			triggerEvent(MODE_TRANSITION_FAILED, PowerSwitchIF::SWITCH_TIMEOUT,
					0);
			setMode(_MODE_POWER_DOWN);
			callChildStatemachine();
			break;
		}
		ReturnValue_t switchState = getStateOfSwitches();
		if ((switchState == PowerSwitchIF::SWITCH_ON)
				|| (switchState == NO_SWITCH)) {
			//NOTE: TransitionSourceMode and -SubMode are set by handleCommandedModeTransition
			childTransitionFailure = CHILD_TIMEOUT;
			setMode(_MODE_START_UP);
			callChildStatemachine();
		}
	}
		break;
	case _MODE_WAIT_OFF: {
		uint32_t currentUptime;
		Clock::getUptime(&currentUptime);

		if(powerSwitcher == nullptr) {
		    setMode(MODE_OFF);
		    break;
		}

		if (currentUptime - timeoutStart >= powerSwitcher->getSwitchDelayMs()) {
			triggerEvent(MODE_TRANSITION_FAILED, PowerSwitchIF::SWITCH_TIMEOUT,
					0);
			setMode(MODE_ERROR_ON);
			break;
		}
		ReturnValue_t switchState = getStateOfSwitches();
		if ((switchState == PowerSwitchIF::SWITCH_OFF)
				|| (switchState == NO_SWITCH)) {
			setMode(_MODE_SWITCH_IS_OFF);
		}
	}
		break;
	case MODE_OFF:
		doOffActivity();
		break;
	case MODE_ON:
		doOnActivity();
		break;
	case MODE_RAW:
	case MODE_NORMAL:
	case MODE_ERROR_ON:
		break;
	case _MODE_SWITCH_IS_OFF:
		setMode(MODE_OFF, SUBMODE_NONE);
		break;
	default:
		triggerEvent(OBJECT_IN_INVALID_MODE, mode, submode);
		setMode(_MODE_POWER_DOWN, 0);
		break;
	}
}

ReturnValue_t DeviceHandlerBase::isModeCombinationValid(Mode_t mode,
		Submode_t submode) {
	switch (mode) {
	case MODE_OFF:
	case MODE_ON:
	case MODE_NORMAL:
	case MODE_RAW:
		if (submode == SUBMODE_NONE) {
			return RETURN_OK;
		} else {
			return INVALID_SUBMODE;
		}
	default:
		return HasModesIF::INVALID_MODE;
	}
}

ReturnValue_t DeviceHandlerBase::insertInCommandAndReplyMap(
		DeviceCommandId_t deviceCommand, uint16_t maxDelayCycles,
		size_t replyLen, bool periodic, bool hasDifferentReplyId,
		DeviceCommandId_t replyId) {
	//No need to check, as we may try to insert multiple times.
	insertInCommandMap(deviceCommand);
	if (hasDifferentReplyId) {
		return insertInReplyMap(replyId, maxDelayCycles, replyLen, periodic);
	} else {
		return insertInReplyMap(deviceCommand, maxDelayCycles, replyLen, periodic);
	}
}

ReturnValue_t DeviceHandlerBase::insertInReplyMap(DeviceCommandId_t replyId,
		uint16_t maxDelayCycles, size_t replyLen, bool periodic) {
	DeviceReplyInfo info;
	info.maxDelayCycles = maxDelayCycles;
	info.periodic = periodic;
	info.delayCycles = 0;
	info.replyLen = replyLen;
	info.command = deviceCommandMap.end();
	auto resultPair = deviceReplyMap.emplace(replyId, info);
	if (resultPair.second) {
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

ReturnValue_t DeviceHandlerBase::insertInCommandMap(
		DeviceCommandId_t deviceCommand) {
	DeviceCommandInfo info;
	info.expectedReplies = 0;
	info.isExecuting = false;
	info.sendReplyTo = NO_COMMANDER;
	auto resultPair = deviceCommandMap.emplace(deviceCommand, info);
	if (resultPair.second) {
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

ReturnValue_t DeviceHandlerBase::updateReplyMapEntry(DeviceCommandId_t deviceReply,
		uint16_t delayCycles, uint16_t maxDelayCycles, bool periodic) {
	std::map<DeviceCommandId_t, DeviceReplyInfo>::iterator iter =
			deviceReplyMap.find(deviceReply);
	if (iter == deviceReplyMap.end()) {
		triggerEvent(INVALID_DEVICE_COMMAND, deviceReply);
		return RETURN_FAILED;
	} else {
		DeviceReplyInfo *info = &(iter->second);
		if (maxDelayCycles != 0) {
			info->maxDelayCycles = maxDelayCycles;
		}
		info->delayCycles = delayCycles;
		info->periodic = periodic;
		return RETURN_OK;
	}
}

void DeviceHandlerBase::callChildStatemachine() {
	if (mode == _MODE_START_UP) {
		doStartUp();
	} else if (mode == _MODE_SHUT_DOWN) {
		doShutDown();
	} else if (mode & TRANSITION_MODE_CHILD_ACTION_MASK) {
		doTransition(transitionSourceMode, transitionSourceSubMode);
	}
}

void DeviceHandlerBase::setTransition(Mode_t modeTo, Submode_t submodeTo) {
	triggerEvent(CHANGING_MODE, modeTo, submodeTo);
	childTransitionDelay = getTransitionDelayMs(mode, modeTo);
	transitionSourceMode = mode;
	transitionSourceSubMode = submode;
	childTransitionFailure = CHILD_TIMEOUT;

	// transitionTargetMode is set by setMode
	setMode((modeTo | TRANSITION_MODE_CHILD_ACTION_MASK), submodeTo);
}

void DeviceHandlerBase::setMode(Mode_t newMode, uint8_t newSubmode) {
	changeHK(mode, submode, false);
	submode = newSubmode;
	mode = newMode;
	modeChanged();
	setNormalDatapoolEntriesInvalid();
	if (!isTransitionalMode()) {
		modeHelper.modeChanged(newMode, newSubmode);
		announceMode(false);
	}
	Clock::getUptime(&timeoutStart);

	if (mode == MODE_OFF) {
		DataSet mySet;
		db_int8_t thermalRequest(deviceThermalRequestPoolId, &mySet,
				PoolVariableIF::VAR_READ_WRITE);
		mySet.read();
		if (thermalRequest != ThermalComponentIF::STATE_REQUEST_IGNORE) {
			thermalRequest = ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL;
		}
		mySet.commit(PoolVariableIF::VALID);
	}
	changeHK(mode, submode, true);
}

void DeviceHandlerBase::setMode(Mode_t newMode) {
	setMode(newMode, submode);
}

void DeviceHandlerBase::replyReturnvalueToCommand(ReturnValue_t status,
		uint32_t parameter) {
	//This is actually the reply protocol for raw and misc DH commands.
	if (status == RETURN_OK) {
		CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, parameter);
		commandQueue->reply(&reply);
	} else {
		CommandMessage reply(CommandMessage::REPLY_REJECTED, status, parameter);
		commandQueue->reply(&reply);
	}
}

void DeviceHandlerBase::replyToCommand(ReturnValue_t status,
		uint32_t parameter) {
//Check if we reply to a raw command.
	if (cookieInfo.pendingCommand->first == RAW_COMMAND_ID) {
		if (status == NO_REPLY_EXPECTED) {
			status = RETURN_OK;
		}
		replyReturnvalueToCommand(status, parameter);
		//Always delete data from a raw command.
		IPCStore->deleteData(storedRawData);
		return;
	}
//Check if we were externally commanded.
	if (cookieInfo.pendingCommand->second.sendReplyTo != NO_COMMANDER) {
		MessageQueueId_t queueId = cookieInfo.pendingCommand->second.sendReplyTo;
		if (status == NO_REPLY_EXPECTED) {
			actionHelper.finish(queueId, cookieInfo.pendingCommand->first,
					RETURN_OK);
		} else {
			actionHelper.step(1, queueId, cookieInfo.pendingCommand->first,
					status);
		}
	}
}

void DeviceHandlerBase::replyToReply(DeviceReplyMap::iterator iter,
		ReturnValue_t status) {
//No need to check if iter exists, as this is checked by callers. If someone else uses the method, add check.
	if (iter->second.command == deviceCommandMap.end()) {
		//Is most likely periodic reply. Silent return.
		return;
	}
//Check if more replies are expected. If so, do nothing.
	DeviceCommandInfo* info = &(iter->second.command->second);
	if (--info->expectedReplies == 0) {
		//Check if it was transition or internal command. Don't send any replies in that case.
		if (info->sendReplyTo != NO_COMMANDER) {
			actionHelper.finish(info->sendReplyTo, iter->first, status);
		}
		info->isExecuting = false;
	}
}

void DeviceHandlerBase::doSendWrite() {
	if (cookieInfo.state == COOKIE_WRITE_READY) {

		ReturnValue_t result = communicationInterface->sendMessage(comCookie,
				rawPacket, rawPacketLen);

		if (result == RETURN_OK) {
			cookieInfo.state = COOKIE_WRITE_SENT;
		} else {
			//always generate a failure event, so that FDIR knows what's up
			triggerEvent(DEVICE_SENDING_COMMAND_FAILED, result,
					cookieInfo.pendingCommand->first);
			replyToCommand(result);
			cookieInfo.state = COOKIE_UNUSED;
			cookieInfo.pendingCommand->second.isExecuting = false;
		}
	}
}

void DeviceHandlerBase::doGetWrite() {
	if (cookieInfo.state != COOKIE_WRITE_SENT) {
		return;
	}
	cookieInfo.state = COOKIE_UNUSED;
	ReturnValue_t result = communicationInterface->getSendSuccess(comCookie);
	if (result == RETURN_OK) {
		if (wiretappingMode == RAW) {
			replyRawData(rawPacket, rawPacketLen, requestedRawTraffic, true);
		}

		//We need to distinguish here, because a raw command never expects a reply.
		//(Could be done in eRIRM, but then child implementations need to be careful.
		result = enableReplyInReplyMap(cookieInfo.pendingCommand);
	} else {
		//always generate a failure event, so that FDIR knows what's up
		triggerEvent(DEVICE_SENDING_COMMAND_FAILED, result,
				cookieInfo.pendingCommand->first);
	}
	if (result != RETURN_OK) {
		cookieInfo.pendingCommand->second.isExecuting = false;
	}
	replyToCommand(result);
}

void DeviceHandlerBase::doSendRead() {
	ReturnValue_t result;

	size_t requestLen = 0;
	if(cookieInfo.pendingCommand != deviceCommandMap.end()) {
		DeviceReplyIter iter = deviceReplyMap.find(
				cookieInfo.pendingCommand->first);
		if(iter != deviceReplyMap.end()) {
			requestLen = iter->second.replyLen;
		}
	}

	result = communicationInterface->requestReceiveMessage(comCookie, requestLen);

	if (result == RETURN_OK) {
		cookieInfo.state = COOKIE_READ_SENT;
	} else {
		triggerEvent(DEVICE_REQUESTING_REPLY_FAILED, result);
		//We can't inform anyone, because we don't know which command was sent last.
		//So, we need to wait for a timeout.
		//but I think we can allow to ignore one missedReply.
		ignoreMissedRepliesCount++;
		cookieInfo.state = COOKIE_UNUSED;
	}
}

void DeviceHandlerBase::doGetRead() {
	size_t receivedDataLen = 0;
	uint8_t *receivedData = nullptr;

	if (cookieInfo.state != COOKIE_READ_SENT) {
		cookieInfo.state = COOKIE_UNUSED;
		return;
	}

	cookieInfo.state = COOKIE_UNUSED;

	ReturnValue_t result = communicationInterface->readReceivedMessage(
			comCookie, &receivedData, &receivedDataLen);

	if (result != RETURN_OK) {
		triggerEvent(DEVICE_REQUESTING_REPLY_FAILED, result);
		//I think we can allow to ignore one missedReply.
		ignoreMissedRepliesCount++;
		return;
	}

	if (receivedDataLen == 0 or result == DeviceCommunicationIF::NO_REPLY_RECEIVED)
		return;

	if (wiretappingMode == RAW) {
		replyRawData(receivedData, receivedDataLen, requestedRawTraffic);
	}

	if (mode == MODE_RAW and defaultRawReceiver != MessageQueueIF::NO_QUEUE) {
		replyRawReplyIfnotWiretapped(receivedData, receivedDataLen);
	}
	else {
		parseReply(receivedData, receivedDataLen);
	}
}

void DeviceHandlerBase::parseReply(const uint8_t* receivedData,
		size_t receivedDataLen) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	DeviceCommandId_t foundId = 0xFFFFFFFF;
	size_t foundLen = 0;
	// The loop may not execute more often than the number of received bytes
	// (worst case). This approach avoids infinite loops due to buggy
	// scanForReply routines.
	uint32_t remainingLength = receivedDataLen;
	for (uint32_t count = 0; count < receivedDataLen; count++) {
		result = scanForReply(receivedData, remainingLength, &foundId,
				&foundLen);
		switch (result) {
		case RETURN_OK:
			handleReply(receivedData, foundId, foundLen);
			break;
		case APERIODIC_REPLY: {
			result = interpretDeviceReply(foundId, receivedData);
			if (result != RETURN_OK) {
				replyRawReplyIfnotWiretapped(receivedData, foundLen);
				triggerEvent(DEVICE_INTERPRETING_REPLY_FAILED, result,
						foundId);
			}
		}
		break;
		case IGNORE_REPLY_DATA:
			break;
		case IGNORE_FULL_PACKET:
			return;
		default:
			//We need to wait for timeout.. don't know what command failed and who sent it.
			replyRawReplyIfnotWiretapped(receivedData, foundLen);
			triggerEvent(DEVICE_READING_REPLY_FAILED, result, foundLen);
			break;
		}
		receivedData += foundLen;
		if (remainingLength > foundLen) {
			remainingLength -= foundLen;
		} else {
			return;
		}
	}
}

void DeviceHandlerBase::handleReply(const uint8_t* receivedData,
		DeviceCommandId_t foundId, uint32_t foundLen) {
	ReturnValue_t result;
	DeviceReplyMap::iterator iter = deviceReplyMap.find(foundId);

	if (iter == deviceReplyMap.end()) {
		replyRawReplyIfnotWiretapped(receivedData, foundLen);
		triggerEvent(DEVICE_UNKNOWN_REPLY, foundId);
		return;
	}

	DeviceReplyInfo *info = &(iter->second);

	if (info->delayCycles != 0) {

		if (info->periodic != false) {
			info->delayCycles = info->maxDelayCycles;
		}
		else {
			info->delayCycles = 0;
		}

		result = interpretDeviceReply(foundId, receivedData);

		if (result != RETURN_OK) {
			// Report failed interpretation to FDIR.
			replyRawReplyIfnotWiretapped(receivedData, foundLen);
			triggerEvent(DEVICE_INTERPRETING_REPLY_FAILED, result, foundId);
		}
		replyToReply(iter, result);
	}
	else {
		// Other completion failure messages are created by timeout.
		// Powering down the device might take some time during which periodic
		// replies may still come in.
		if (mode != _MODE_WAIT_OFF) {
			triggerEvent(DEVICE_UNREQUESTED_REPLY, foundId);
		}
	}
}

ReturnValue_t DeviceHandlerBase::getStorageData(store_address_t storageAddress,
		uint8_t** data, uint32_t * len) {
	size_t lenTmp;

	if (IPCStore == nullptr) {
		*data = nullptr;
		*len = 0;
		return RETURN_FAILED;
	}
	ReturnValue_t result = IPCStore->modifyData(storageAddress, data, &lenTmp);
	if (result == RETURN_OK) {
		*len = lenTmp;
		return RETURN_OK;
	} else {
		triggerEvent(StorageManagerIF::GET_DATA_FAILED, result,
				storageAddress.raw);
		*data = nullptr;
		*len = 0;
		return result;
	}
}

void DeviceHandlerBase::replyRawData(const uint8_t *data, size_t len,
		MessageQueueId_t sendTo, bool isCommand) {
	if (IPCStore == nullptr or len == 0 or sendTo == MessageQueueIF::NO_QUEUE) {
		return;
	}
	store_address_t address;
	ReturnValue_t result = IPCStore->addData(&address, data, len);

	if (result != RETURN_OK) {
		triggerEvent(StorageManagerIF::STORE_DATA_FAILED, result);
		return;
	}

	CommandMessage command;

	DeviceHandlerMessage::setDeviceHandlerRawReplyMessage(&command,
			getObjectId(), address, isCommand);

	result = commandQueue->sendMessage(sendTo, &command);

	if (result != RETURN_OK) {
		IPCStore->deleteData(address);
		// Silently discard data, this indicates heavy TM traffic which
		// should not be increased by additional events.
	}
}

//Default child implementations
DeviceHandlerIF::CommunicationAction_t DeviceHandlerBase::getComAction() {
	switch (pstStep) {
	case 0:
		return SEND_WRITE;
		break;
	case 1:
		return GET_WRITE;
		break;
	case 2:
		return SEND_READ;
		break;
	case 3:
		return GET_READ;
		break;
	default:
		break;
	}
	return NOTHING;
}

MessageQueueId_t DeviceHandlerBase::getCommandQueue() const {
	return commandQueue->getId();
}

void DeviceHandlerBase::buildRawDeviceCommand(CommandMessage* commandMessage) {
	storedRawData = DeviceHandlerMessage::getStoreAddress(commandMessage);
	ReturnValue_t result = getStorageData(storedRawData, &rawPacket,
			&rawPacketLen);
	if (result != RETURN_OK) {
		replyReturnvalueToCommand(result, RAW_COMMAND_ID);
		storedRawData.raw = StorageManagerIF::INVALID_ADDRESS;
	} else {
		cookieInfo.pendingCommand = deviceCommandMap.find(
				(DeviceCommandId_t) RAW_COMMAND_ID);
		cookieInfo.pendingCommand->second.isExecuting = true;
		cookieInfo.state = COOKIE_WRITE_READY;
	}
}

void DeviceHandlerBase::commandSwitch(ReturnValue_t onOff) {
	if(powerSwitcher == nullptr) {
		return;
	}
	const uint8_t *switches;
	uint8_t numberOfSwitches = 0;
	ReturnValue_t result = getSwitches(&switches, &numberOfSwitches);
	if (result == RETURN_OK) {
		while (numberOfSwitches > 0) {
			powerSwitcher->sendSwitchCommand(switches[numberOfSwitches - 1],
					onOff);
			numberOfSwitches--;
		}
	}
}

ReturnValue_t DeviceHandlerBase::getSwitches(const uint8_t **switches,
		uint8_t *numberOfSwitches) {
	return DeviceHandlerBase::NO_SWITCH;
}

void DeviceHandlerBase::modeChanged(void) {
}

ReturnValue_t DeviceHandlerBase::enableReplyInReplyMap(
		DeviceCommandMap::iterator command, uint8_t expectedReplies,
		bool useAlternativeId, DeviceCommandId_t alternativeReply) {
	DeviceReplyMap::iterator iter;
	if (useAlternativeId) {
		iter = deviceReplyMap.find(alternativeReply);
	} else {
		iter = deviceReplyMap.find(command->first);
	}
	if (iter != deviceReplyMap.end()) {
		DeviceReplyInfo *info = &(iter->second);
		info->delayCycles = info->maxDelayCycles;
		info->command = command;
		command->second.expectedReplies = expectedReplies;
		return RETURN_OK;
	} else {
		return NO_REPLY_EXPECTED;
	}
}

void DeviceHandlerBase::doTransition(Mode_t modeFrom, Submode_t subModeFrom) {
	setMode(getBaseMode(mode));
}

uint32_t DeviceHandlerBase::getTransitionDelayMs(Mode_t modeFrom,
		Mode_t modeTo) {
	return 0;
}

ReturnValue_t DeviceHandlerBase::getStateOfSwitches(void) {
	if(powerSwitcher == nullptr) {
		return NO_SWITCH;
	}
	uint8_t numberOfSwitches = 0;
	const uint8_t *switches;

	ReturnValue_t result = getSwitches(&switches, &numberOfSwitches);
	if ((result == RETURN_OK) && (numberOfSwitches != 0)) {
		while (numberOfSwitches > 0) {
			if (powerSwitcher->getSwitchState(switches[numberOfSwitches - 1])
					== PowerSwitchIF::SWITCH_OFF) {
				return PowerSwitchIF::SWITCH_OFF;
			}
			numberOfSwitches--;
		}
		return PowerSwitchIF::SWITCH_ON;
	}

	return NO_SWITCH;
}

Mode_t DeviceHandlerBase::getBaseMode(Mode_t transitionMode) {
//only child action special modes are handled, as a child should never see any base action modes
	if (transitionMode == _MODE_START_UP) {
		return _MODE_TO_ON;
	}
	if (transitionMode == _MODE_SHUT_DOWN) {
		return _MODE_POWER_DOWN;
	}
	return transitionMode
			& ~(TRANSITION_MODE_BASE_ACTION_MASK
					| TRANSITION_MODE_CHILD_ACTION_MASK);
}

//SHOULDDO: Allow transition from OFF to NORMAL to reduce complexity in assemblies. And, by the way, throw away DHB and write a new one:
// - Include power and thermal completely, but more modular :-)
// - Don't use modes for state transitions, reduce FSM (Finte State Machine) complexity.
// - Modularization?
ReturnValue_t DeviceHandlerBase::checkModeCommand(Mode_t commandedMode,
		Submode_t commandedSubmode, uint32_t* msToReachTheMode) {
	if (isTransitionalMode()) {
		return IN_TRANSITION;
	}
	if ((mode == MODE_ERROR_ON) && (commandedMode != MODE_OFF)) {
		return TRANS_NOT_ALLOWED;
	}
	if ((commandedMode == MODE_NORMAL) && (mode == MODE_OFF)) {
		return TRANS_NOT_ALLOWED;
	}

	if ((commandedMode == MODE_ON) && (mode == MODE_OFF)
			&& (deviceThermalStatePoolId != PoolVariableIF::NO_PARAMETER)) {
		DataSet mySet;
		db_int8_t thermalState(deviceThermalStatePoolId, &mySet,
				PoolVariableIF::VAR_READ);
		db_int8_t thermalRequest(deviceThermalRequestPoolId, &mySet,
				PoolVariableIF::VAR_READ);
		mySet.read();
		if (thermalRequest != ThermalComponentIF::STATE_REQUEST_IGNORE) {
			if (!ThermalComponentIF::isOperational(thermalState)) {
				triggerEvent(ThermalComponentIF::TEMP_NOT_IN_OP_RANGE,
						thermalState);
				return NON_OP_TEMPERATURE;
			}
		}
	}

	return isModeCombinationValid(commandedMode, commandedSubmode);
}

void DeviceHandlerBase::startTransition(Mode_t commandedMode,
		Submode_t commandedSubmode) {
	switch (commandedMode) {
	case MODE_ON:
		if (mode == MODE_OFF) {
			transitionSourceMode = _MODE_POWER_DOWN;
			transitionSourceSubMode = SUBMODE_NONE;
			setMode(_MODE_POWER_ON, commandedSubmode);
			//already set the delay for the child transition so we don't need to call it twice
			childTransitionDelay = getTransitionDelayMs(_MODE_START_UP,
					MODE_ON);
			triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
			DataSet mySet;
			db_int8_t thermalRequest(deviceThermalRequestPoolId,
					&mySet, PoolVariableIF::VAR_READ_WRITE);
			mySet.read();
			if (thermalRequest != ThermalComponentIF::STATE_REQUEST_IGNORE) {
				thermalRequest = ThermalComponentIF::STATE_REQUEST_OPERATIONAL;
				mySet.commit(PoolVariableIF::VALID);
			}
		} else {
			setTransition(MODE_ON, commandedSubmode);
		}
		break;
	case MODE_OFF:
		if (mode == MODE_OFF) {
			triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
			setMode(_MODE_POWER_DOWN, commandedSubmode);
		} else {
			//already set the delay for the child transition so we don't need to call it twice
			childTransitionDelay = getTransitionDelayMs(mode, _MODE_POWER_DOWN);
			transitionSourceMode = _MODE_POWER_DOWN;
			transitionSourceSubMode = commandedSubmode;
			childTransitionFailure = CHILD_TIMEOUT;
			setMode(_MODE_SHUT_DOWN, commandedSubmode);
			triggerEvent(CHANGING_MODE, commandedMode, commandedSubmode);
		}
		break;
	case MODE_RAW:
		if (mode != MODE_OFF) {
			setTransition(MODE_RAW, commandedSubmode);
		} else {
			setMode(MODE_RAW, commandedSubmode);
		}
		break;
	case MODE_NORMAL:
		if (mode != MODE_OFF) {
			setTransition(MODE_NORMAL, commandedSubmode);
		} else {
			replyReturnvalueToCommand(HasModesIF::TRANS_NOT_ALLOWED);
		}
		break;
	}
}

void DeviceHandlerBase::getMode(Mode_t* mode, Submode_t* submode) {
	*mode = this->mode;
	*submode = this->submode;
}

void DeviceHandlerBase::setToExternalControl() {
	healthHelper.setHealth(EXTERNAL_CONTROL);
}

void DeviceHandlerBase::announceMode(bool recursive) {
	triggerEvent(MODE_INFO, mode, submode);
}

bool DeviceHandlerBase::dontCheckQueue() {
	return false;
}

void DeviceHandlerBase::missedReply(DeviceCommandId_t id) {
	if (ignoreMissedRepliesCount > 0) {
		ignoreMissedRepliesCount--;
	} else {
		triggerEvent(DEVICE_MISSED_REPLY, id);
	}
}

HasHealthIF::HealthState DeviceHandlerBase::getHealth() {
	return healthHelper.getHealth();
}

ReturnValue_t DeviceHandlerBase::setHealth(HealthState health) {
    healthHelper.setHealth(health);
    return HasReturnvaluesIF::RETURN_OK;
}

void DeviceHandlerBase::checkSwitchState() {
	if ((mode == MODE_ON || mode == MODE_NORMAL)) {
		//We only check in ON and NORMAL, ignore RAW and ERROR_ON.
		ReturnValue_t result = getStateOfSwitches();
		if (result == PowerSwitchIF::SWITCH_OFF && !switchOffWasReported) {
			triggerEvent(PowerSwitchIF::SWITCH_WENT_OFF);
			switchOffWasReported = true;
		}
	} else {
		switchOffWasReported = false;
	}
}

void DeviceHandlerBase::doOnActivity() {
}

ReturnValue_t DeviceHandlerBase::acceptExternalDeviceCommands() {
	if ((mode != MODE_ON) && (mode != MODE_NORMAL)) {
		return WRONG_MODE_FOR_COMMAND;
	}
	return RETURN_OK;
}

void DeviceHandlerBase::replyRawReplyIfnotWiretapped(const uint8_t* data,
		size_t len) {
	if ((wiretappingMode == RAW)
			&& (defaultRawReceiver == requestedRawTraffic)) {
		//The raw packet was already sent by the wiretapping service
	} else {
		replyRawData(data, len, defaultRawReceiver);
	}
}

ReturnValue_t DeviceHandlerBase::handleDeviceHandlerMessage(
		CommandMessage * message) {
	switch (message->getCommand()) {
	case DeviceHandlerMessage::CMD_WIRETAPPING:
		switch (DeviceHandlerMessage::getWiretappingMode(message)) {
		case RAW:
			wiretappingMode = RAW;
			requestedRawTraffic = commandQueue->getLastPartner();
			break;
		case TM:
			wiretappingMode = TM;
			requestedRawTraffic = commandQueue->getLastPartner();
			break;
		case OFF:
			wiretappingMode = OFF;
			break;
		default:
			replyReturnvalueToCommand(INVALID_COMMAND_PARAMETER);
			wiretappingMode = OFF;
			return RETURN_OK;
		}
		replyReturnvalueToCommand(RETURN_OK);
		return RETURN_OK;
//	case DeviceHandlerMessage::CMD_SWITCH_IOBOARD:
//		if (mode != MODE_OFF) {
//			replyReturnvalueToCommand(WRONG_MODE_FOR_COMMAND);
//		} else {
//			result = switchCookieChannel(
//					DeviceHandlerMessage::getIoBoardObjectId(message));
//			if (result == RETURN_OK) {
//				replyReturnvalueToCommand(RETURN_OK);
//			} else {
//				replyReturnvalueToCommand(CANT_SWITCH_IO_ADDRESS);
//			}
//		}
//		return RETURN_OK;
	case DeviceHandlerMessage::CMD_RAW:
		if ((mode != MODE_RAW)) {
			DeviceHandlerMessage::clear(message);
			replyReturnvalueToCommand(WRONG_MODE_FOR_COMMAND);
		} else {
			buildRawDeviceCommand(message);
		}
		return RETURN_OK;
	default:
		return RETURN_FAILED;
	}
}

void DeviceHandlerBase::setParentQueue(MessageQueueId_t parentQueueId) {
	modeHelper.setParentQueue(parentQueueId);
	healthHelper.setParentQueue(parentQueueId);
}

bool DeviceHandlerBase::isAwaitingReply() {
	std::map<DeviceCommandId_t, DeviceReplyInfo>::iterator iter;
	for (iter = deviceReplyMap.begin(); iter != deviceReplyMap.end(); ++iter) {
		if (iter->second.delayCycles != 0) {
			return true;
		}
	}
	return false;
}

ReturnValue_t DeviceHandlerBase::letChildHandleMessage(
		CommandMessage * message) {
	return RETURN_FAILED;
}

void DeviceHandlerBase::handleDeviceTM(SerializeIF* data,
		DeviceCommandId_t replyId, bool neverInDataPool, bool forceDirectTm) {
	DeviceReplyMap::iterator iter = deviceReplyMap.find(replyId);
	if (iter == deviceReplyMap.end()) {
		triggerEvent(DEVICE_UNKNOWN_REPLY, replyId);
		return;
	}
	DeviceTmReportingWrapper wrapper(getObjectId(), replyId, data);
	//replies to a command
	if (iter->second.command != deviceCommandMap.end())
	{
		MessageQueueId_t queueId = iter->second.command->second.sendReplyTo;

		if (queueId != NO_COMMANDER) {
			//This may fail, but we'll ignore the fault.
			actionHelper.reportData(queueId, replyId, data);
		}

		//This check should make sure we get any TM but don't get anything doubled.
		if (wiretappingMode == TM && (requestedRawTraffic != queueId)) {
			actionHelper.reportData(requestedRawTraffic, replyId, &wrapper);
		}
		else if (forceDirectTm and (defaultRawReceiver != queueId) and
					(defaultRawReceiver != MessageQueueIF::NO_QUEUE))
		{
			// hiding of sender needed so the service will handle it as
			// unexpected Data, no matter what state (progress or completed)
			// it is in
			actionHelper.reportData(defaultRawReceiver, replyId, &wrapper,
					true);
		}
	}
	//unrequested/aperiodic replies
	else
	{
		if (wiretappingMode == TM) {
			actionHelper.reportData(requestedRawTraffic, replyId, &wrapper);
		}
		else if (forceDirectTm and defaultRawReceiver !=
				MessageQueueIF::NO_QUEUE)
		{
			// hiding of sender needed so the service will handle it as
			// unexpected Data, no matter what state (progress or completed)
			// it is in
			actionHelper.reportData(defaultRawReceiver, replyId, &wrapper,
					true);
		}
	}
	//Try to cast to GlobDataSet and commit data.
	if (!neverInDataPool) {
		DataSet* dataSet = dynamic_cast<DataSet*>(data);
		if (dataSet != NULL) {
			dataSet->commit(PoolVariableIF::VALID);
		}
	}
}

ReturnValue_t DeviceHandlerBase::executeAction(ActionId_t actionId,
		MessageQueueId_t commandedBy, const uint8_t* data, size_t size) {
	ReturnValue_t result = acceptExternalDeviceCommands();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	DeviceCommandMap::iterator iter = deviceCommandMap.find(actionId);
	if (iter == deviceCommandMap.end()) {
		result = COMMAND_NOT_SUPPORTED;
	} else if (iter->second.isExecuting) {
		result = COMMAND_ALREADY_SENT;
	} else {
		result = buildCommandFromCommand(actionId, data, size);
	}
	if (result == RETURN_OK) {
		iter->second.sendReplyTo = commandedBy;
		iter->second.isExecuting = true;
		cookieInfo.pendingCommand = iter;
		cookieInfo.state = COOKIE_WRITE_READY;
	}
	return result;
}

void DeviceHandlerBase::buildInternalCommand(void) {
//Neither Raw nor Direct could build a command
	ReturnValue_t result = NOTHING_TO_SEND;
	DeviceCommandId_t deviceCommandId = NO_COMMAND_ID;
	if (mode == MODE_NORMAL) {
		result = buildNormalDeviceCommand(&deviceCommandId);
		if (result == BUSY) {
		    //so we can track misconfigurations
			sif::debug << std::hex << getObjectId()
					<< ": DHB::buildInternalCommand: Busy" << std::dec << std::endl;
			result = NOTHING_TO_SEND; //no need to report this
		}
	}
	else if (mode == MODE_RAW) {
		result = buildChildRawCommand();
		deviceCommandId = RAW_COMMAND_ID;
	}
	else if (mode & TRANSITION_MODE_CHILD_ACTION_MASK) {
		result = buildTransitionDeviceCommand(&deviceCommandId);
	}
	else {
		return;
	}

	if (result == NOTHING_TO_SEND) {
		return;
	}
	if (result == RETURN_OK) {
		DeviceCommandMap::iterator iter = deviceCommandMap.find(
				deviceCommandId);
		if (iter == deviceCommandMap.end()) {
			result = COMMAND_NOT_SUPPORTED;
		} else if (iter->second.isExecuting) {
			sif::debug << std::hex << getObjectId()
					<< ": DHB::buildInternalCommand: Command "
					<< deviceCommandId << " isExecuting" << std::endl; //so we can track misconfigurations
			return; //this is an internal command, no need to report a failure here, missed reply will track if a reply is too late, otherwise, it's ok
		} else {
			iter->second.sendReplyTo = NO_COMMANDER;
			iter->second.isExecuting = true;
			cookieInfo.pendingCommand = iter;
			cookieInfo.state = COOKIE_WRITE_READY;
		}
	}
	if (result != RETURN_OK) {
		triggerEvent(DEVICE_BUILDING_COMMAND_FAILED, result, deviceCommandId);
	}
}

ReturnValue_t DeviceHandlerBase::buildChildRawCommand() {
	return NOTHING_TO_SEND;
}

uint8_t DeviceHandlerBase::getReplyDelayCycles(
		DeviceCommandId_t deviceCommand) {
	DeviceReplyMap::iterator iter = deviceReplyMap.find(deviceCommand);
	if (iter == deviceReplyMap.end()) {
		return 0;
	}
	return iter->second.delayCycles;
}

Mode_t DeviceHandlerBase::getTransitionSourceMode() const {
	return transitionSourceMode;
}

Submode_t DeviceHandlerBase::getTransitionSourceSubMode() const {
	return transitionSourceSubMode;
}

void DeviceHandlerBase::triggerEvent(Event event, uint32_t parameter1,
		uint32_t parameter2) {
	fdirInstance->triggerEvent(event, parameter1, parameter2);
}

void DeviceHandlerBase::forwardEvent(Event event, uint32_t parameter1,
		uint32_t parameter2) const {
	fdirInstance->triggerEvent(event, parameter1, parameter2);
}

void DeviceHandlerBase::doOffActivity() {
}

ReturnValue_t DeviceHandlerBase::getParameter(uint8_t domainId,
		uint16_t parameterId, ParameterWrapper* parameterWrapper,
		const ParameterWrapper* newValues, uint16_t startAtIndex) {
	ReturnValue_t result = fdirInstance->getParameter(domainId, parameterId,
			parameterWrapper, newValues, startAtIndex);
	if (result != INVALID_DOMAIN_ID) {
		return result;
	}
	return INVALID_DOMAIN_ID;

}

bool DeviceHandlerBase::isTransitionalMode() {
	return ((mode
			& (TRANSITION_MODE_BASE_ACTION_MASK
					| TRANSITION_MODE_CHILD_ACTION_MASK)) != 0);
}

bool DeviceHandlerBase::commandIsExecuting(DeviceCommandId_t commandId) {
	auto iter = deviceCommandMap.find(commandId);
	if (iter != deviceCommandMap.end()) {
		return iter->second.isExecuting;
	} else {
		return false;
	}

}

void DeviceHandlerBase::changeHK(Mode_t mode, Submode_t submode, bool enable) {
}

void DeviceHandlerBase::setTaskIF(PeriodicTaskIF* task_){
	executingTask = task_;
}

// Default implementations empty.
void DeviceHandlerBase::debugInterface(uint8_t positionTracker,
		object_id_t objectId, uint32_t parameter) {}

void DeviceHandlerBase::performOperationHook() {
}

ReturnValue_t DeviceHandlerBase::initializeAfterTaskCreation() {
    // In this function, the task handle should be valid if the task
    // was implemented correctly. We still check to be 1000 % sure :-)
    if(executingTask != nullptr) {
        pstIntervalMs = executingTask->getPeriodMs();
    }
    return HasReturnvaluesIF::RETURN_OK;
}

