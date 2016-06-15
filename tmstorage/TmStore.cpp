/*
 * TmStore.cpp
 *
 *  Created on: 05.02.2015
 *      Author: baetz
 */

#include <framework/memory/AcceptsMemoryMessagesIF.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmstorage/TmStore.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tmtcservices/TmTcMessage.h>


TmStore::TmStore(TmStoreFrontendIF* owner, object_id_t memoryObject,
		uint32_t storeAddress, uint32_t size, bool overwriteOld,
		uint32_t maxDumpPacketsPerCylce, uint32_t maxDumpedBytesPerCylce,
		uint32_t updateRemotePtrsMs, uint32_t chunkSize) :
		owner(owner), eventProxy(NULL), info(&oldestPacket, &newestPacket), infoSize(
				info.getSerializedSize()), ring(storeAddress + infoSize,
				size - infoSize, overwriteOld), state(OFF), writeState(
				WRITE_IDLE), readState(READ_IDLE), memoryObject(memoryObject), memoryQueue(), ipcStore(
		NULL), timer(), pendingDataToWrite(0), maximumAmountToRead(0), storedPacketCounter(0), pendingStoredPackets(0), splitWrite(
		false), splitRead(NO_SPLIT), splitReadTotalSize(0), tmBuffer(chunkSize), dumpBuffer(
		NULL), pointerAddress(storeAddress), updateRemotePtrsMs(
				updateRemotePtrsMs), maxDumpPacketsPerCycle(
				maxDumpPacketsPerCylce), maxDumpedBytesPerCycle(maxDumpedBytesPerCylce), tryToSetStoreInfo(false) {
	dumpBuffer = new uint8_t[chunkSize];
}

TmStore::~TmStore() {
	delete[] dumpBuffer;
}

ReturnValue_t TmStore::performOperation() {
	doStateMachine();
	checkMemoryQueue();
	if (localBufferTimer.hasTimedOut() && (state == READY)) {
		sendTmBufferToStore();
		localBufferTimer.setTimeout(LOCAL_BUFFER_TIMEOUT_MS);
	}
	return RETURN_OK;
}

ReturnValue_t TmStore::storePacket(TmPacketMinimal* tmPacket) {
	if (tmPacket->getFullSize() > chunkSize()) {
		return TOO_LARGE;
	}
	ReturnValue_t result = RETURN_FAILED;
	if (hasEnoughSpaceFor(tmPacket->getFullSize())) {
		localBufferTimer.setTimeout(LOCAL_BUFFER_TIMEOUT_MS);
		result = storeOrForwardPacket(tmPacket->getWholeData(),
				tmPacket->getFullSize());
	} else {
		result = handleFullStore(tmPacket->getWholeData(),
				tmPacket->getFullSize());
	}
	if (result == RETURN_OK) {
		pendingNewestPacket.setContent(tmPacket);
		if (!pendingOldestPacket.isValid()) {
			pendingOldestPacket.setContent(tmPacket);
		}
		pendingStoredPackets++;
	}
	return result;
}

ReturnValue_t TmStore::fetchPackets(bool useAddress, uint32_t startAtAddress) {
	if (!isReady()) {
		return NOT_READY;
	}
	if (ring.isEmpty()) {
		return EMPTY;
	}
	if (readState != READ_IDLE) {
		return BUSY;
	}
	// Never download more than the currently available data.
	maximumAmountToRead = ring.availableReadData();
	if (useAddress) {
		ring.setRead(startAtAddress, TEMP_READ_PTR);
	} else {
		ring.setRead(ring.getRead(READ_PTR), TEMP_READ_PTR);
	}
	ReturnValue_t result = requestChunkOfData();
	if (result != RETURN_OK) {
		return result;
	}
	setReadState(DUMPING_PACKETS);
	return RETURN_OK;
}

ReturnValue_t TmStore::requestStoreInfo() {
	CommandMessage message;
	MemoryMessage::setMemoryDumpCommand(&message, pointerAddress, infoSize);
	ReturnValue_t result = memoryQueue.sendToDefault(&message);
	if (result != RETURN_OK) {
		return result;
	}
	return RETURN_OK;
}

ReturnValue_t TmStore::setStoreInfo() {
	store_address_t storeId;
	uint8_t* data = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getFreeElement(&storeId, infoSize, &data);
	if (result != RETURN_OK) {
		return result;
	}
	info.setContent(ring.getRead(), ring.getWrite(), storedPacketCounter);
	result = info.serialize(&data, &size, infoSize, true);
	if (result != RETURN_OK) {
		ipcStore->deleteData(storeId);
		return result;
	}
	CommandMessage message;
	MemoryMessage::setMemoryLoadCommand(&message, pointerAddress, storeId);
	result = memoryQueue.sendToDefault(&message);
	if (result != RETURN_OK) {
		ipcStore->deleteData(storeId);
		return result;
	}
	return RETURN_OK;
}

void TmStore::doStateMachine() {
	switch (state) {
	case OFF:
		break;
	case STARTUP: {
		ReturnValue_t result = requestStoreInfo();
		if (result == RETURN_OK) {
			setState(FETCH_STORE_INFORMATION);
			setReadState(FETCHING_STORE_INFO);
			timer.setTimeout(DEFAULT_TIMEOUT_MS);
		} else {
			eventProxy->forwardEvent(STORE_INIT_FAILED, result, 0);
			setState(OFF);
		}
		}
		break;
	case FETCH_STORE_INFORMATION:
		if (timer.hasTimedOut()) {
			eventProxy->forwardEvent(STORE_INIT_FAILED, TIMEOUT, 1);
			setState(OFF);
		}
		break;
	case READY:
		if (tryToSetStoreInfo) {
			if ((writeState == WRITE_IDLE) && (readState == READ_IDLE)) {
				if (setStoreInfo() == RETURN_OK) {
					setWriteState(SETTING_STORE_INFO);
				}
				tryToSetStoreInfo = false;
			}
		}
		break;
	default:
		//do nothing.
		break;
	}
}

void TmStore::checkMemoryQueue() {
	CommandMessage message;
	for (ReturnValue_t result = memoryQueue.receiveMessage(&message);
			result == RETURN_OK;
			result = memoryQueue.receiveMessage(&message)) {
		switch (message.getCommand()) {
		case CommandMessage::REPLY_COMMAND_OK:
			handleLoadSuccess();
			break;
		case MemoryMessage::REPLY_MEMORY_DUMP:
			handleDump(MemoryMessage::getStoreID(&message));
			break;
		case MemoryMessage::REPLY_MEMORY_FAILED:
		case CommandMessage::REPLY_REJECTED: // REPLY_REJECTED uses the same protocol.
			switch (MemoryMessage::getInitialCommand(&message)) {
			case MemoryMessage::CMD_MEMORY_LOAD:
				handleLoadFailed(MemoryMessage::getErrorCode(&message));
				break;
			case MemoryMessage::CMD_MEMORY_DUMP:
				handleDumpFailed(MemoryMessage::getErrorCode(&message));
				break;
			default:
				debug << "TmStore: Unknown InitialCommand: "
						<< MemoryMessage::getInitialCommand(&message)
						<< std::endl;
				break;
			}
			break;
		default:
			eventProxy->forwardEvent(UNEXPECTED_MSG, 0, 0);
			break;
		}
	}
}

void TmStore::handleLoadSuccess() {
	switch (writeState) {
	case SETTING_STORE_INFO:
		setWriteState(WRITE_IDLE);
		break;
	case SENDING_PACKETS:
		if (splitWrite) {
			ReturnValue_t result = sendRemainingTmPiece();
			if (result != RETURN_OK) {
				eventProxy->forwardEvent(STORE_SEND_WRITE_FAILED, result, 0);
				resetStore(true);
				pendingDataToWrite = 0;
			}
			splitWrite = false;
		} else {
			ring.writeData(pendingDataToWrite);
			pendingDataToWrite = 0;
			storedPacketCounter += pendingStoredPackets;
			newestPacket.setContent(&pendingNewestPacket);
			if (!oldestPacket.isValid()) {
				oldestPacket.setContent(&pendingOldestPacket);
			}
			pendingStoredPackets = 0;
			setWriteState(WRITE_IDLE);
			tryToSetStoreInfo = true;
		}
		break;
	default:
		eventProxy->forwardEvent(UNEXPECTED_MSG, 0, 1);
	}
}

void TmStore::handleDump(store_address_t storeId) {
	const uint8_t* buffer = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(storeId, &buffer, &size);
	if (result != RETURN_OK) {
		return;
	}
	switch (readState) {
	case FETCHING_STORE_INFO:
		readStoreInfo(buffer, size);
		break;
	case DUMPING_PACKETS:
	case DELETING_OLD:
	case DELETING_MORE:
		handleSplitRead(buffer, size);
		break;
	default:
		setReadState(READ_IDLE);
		//No break.
	case READ_IDLE:
		eventProxy->forwardEvent(UNEXPECTED_MSG, 0, 2);
		break;
	}
	ipcStore->deleteData(storeId);
}

void TmStore::handleSplitRead(const uint8_t* buffer, uint32_t size) {
	switch (splitRead) {
	case NO_SPLIT:
		if (readState == DELETING_OLD || readState == DELETING_MORE) {
			deleteOld(buffer, size);
		} else {
			dumpPackets(buffer, size);
		}
		break;
	case SPLIT_STARTED:
		if (size <= chunkSize()) {
			ReturnValue_t result = requestRemainingDumpPiece(size);
			if (result == RETURN_OK) {
				memcpy(dumpBuffer, buffer, size);
				splitRead = ONE_RECEIVED;
			} else {
				eventProxy->forwardEvent(STORE_SEND_READ_FAILED, result, 0);
				splitRead = NO_SPLIT;
				maximumAmountToRead = 0;
			}
		} else {
			eventProxy->forwardEvent(STORE_SEND_READ_FAILED, TOO_LARGE, 0);
			splitRead = NO_SPLIT;
			maximumAmountToRead = 0;
		}
		break;
	case ONE_RECEIVED:
		memcpy(&dumpBuffer[splitReadTotalSize - size], buffer, size);
		if (readState == DELETING_OLD || readState == DELETING_MORE) {
			deleteOld(dumpBuffer, splitReadTotalSize);
		} else {
			dumpPackets(dumpBuffer, splitReadTotalSize);
		}
		splitRead = NO_SPLIT;
		break;
	}
}

void TmStore::handleLoadFailed(ReturnValue_t errorCode) {
	eventProxy->forwardEvent(STORE_WRITE_FAILED, errorCode, 0);
	setWriteState(WRITE_IDLE);
	setState(READY);
	splitWrite = false;
	pendingDataToWrite = 0;
}

void TmStore::handleDumpFailed(ReturnValue_t errorCode) {
	switch (readState) {
	case FETCHING_STORE_INFO:
		eventProxy->forwardEvent(STORE_INIT_FAILED, errorCode, 2);
		setState(OFF);
		break;
	case DUMPING_PACKETS:
		owner->handleRetrievalFailed(errorCode);
		//No break
	default:
	case READ_IDLE:
	case DELETING_OLD:
	case DELETING_MORE:
		eventProxy->forwardEvent(STORE_READ_FAILED, errorCode, 0);
		setState(READY);
		break;
	}
	setReadState(READ_IDLE);
	splitRead = NO_SPLIT;
	maximumAmountToRead = 0;
}

void TmStore::readStoreInfo(const uint8_t* buffer, uint32_t size) {
	setReadState(READ_IDLE);
	if (::Calculate_CRC(buffer, infoSize) != 0) {
		eventProxy->forwardEvent(STORE_INIT_EMPTY, 0, 0);
		setState(READY);
		return;
	}
	int32_t deSize = size;
	ReturnValue_t result = info.deSerialize(&buffer, &deSize, true);
	if (result != RETURN_OK) {
		//An error here is extremely unlikely.
		eventProxy->forwardEvent(STORE_INIT_FAILED, result, 3);
		setState(OFF);
		return;
	}
	eventProxy->forwardEvent(STORE_INIT_DONE);
	ring.setWrite(info.writeP);
	ring.setRead(info.readP);
	storedPacketCounter = info.storedPacketCount;
	setState(READY);
}

void TmStore::dumpPackets(const uint8_t* buffer, uint32_t size) {
	ReturnValue_t result = RETURN_OK;
	uint32_t tempSize = 0;
	uint32_t dumpedPacketCounter = 0;
	uint32_t dumpedBytesCounter = 0;
	while (size >= TmPacketMinimal::MINIMUM_SIZE
			&& dumpedPacketCounter < maxDumpPacketsPerCycle
			&& dumpedBytesCounter < maxDumpedBytesPerCycle
			&& result == RETURN_OK) {
		TmPacketMinimal packet(buffer);
		tempSize = packet.getFullSize();
		if (tempSize > size) {
			if (tempSize > ring.availableReadData(READ_PTR) || tempSize > chunkSize()) {
				owner->handleRetrievalFailed(DUMP_ERROR);
				eventProxy->forwardEvent(STORE_CONTENT_CORRUPTED, 0, tempSize);
				result = DUMP_ERROR;
			}
			break;
		}
		size -= tempSize;
		buffer += tempSize;
		dumpedBytesCounter += tempSize;
		if (maximumAmountToRead > tempSize) {
			result = owner->packetRetrieved(&packet,
					ring.getRead(TEMP_READ_PTR));
			maximumAmountToRead -= tempSize;
		} else {
			//packet is complete and last.
			result = owner->packetRetrieved(&packet,
					ring.getRead(TEMP_READ_PTR), true);
			maximumAmountToRead = 0;
			break;
		}
		if (result == RETURN_OK) {
			ring.readData(tempSize, TEMP_READ_PTR);
			dumpedPacketCounter++;
		}
	}
	if (result == RETURN_OK && maximumAmountToRead != 0) {
		result = requestChunkOfData();
		if (result != HasReturnvaluesIF::RETURN_OK) {
			owner->handleRetrievalFailed(result);
		}
	} else {
		setReadState(READ_IDLE);
	}
}

ReturnValue_t TmStore::sendDataToTmStore(const uint8_t* data, uint32_t size) {
	//Unfortunately, cleanup is quite complex.
	store_address_t storeId;
	ReturnValue_t result = RETURN_FAILED;
	if (size > ring.writeTillWrap()) {
		//Two-folded write
		result = ipcStore->addData(&storeId, data, ring.writeTillWrap());
		if (result != RETURN_OK) {
			return result;
		}
		splitWrite = true;
		result = ipcStore->addData(&splitWriteStoreId,
				data + ring.writeTillWrap(), size - ring.writeTillWrap());
		if (result != RETURN_OK) {
			ipcStore->deleteData(storeId);
		}
	} else {
		//Normal write
		splitWrite = false;
		result = ipcStore->addData(&storeId, data, size);
	}
	if (result != RETURN_OK) {
		splitWrite = false;
		return result;
	}
	CommandMessage message;
	MemoryMessage::setMemoryLoadCommand(&message, ring.getWrite(), storeId);
	if ((result = memoryQueue.sendToDefault(&message)) == RETURN_OK) {
		setWriteState(SENDING_PACKETS);
		pendingDataToWrite = size;
	} else {
		if (splitWrite) {
			ipcStore->deleteData(splitWriteStoreId);
		}
		splitWrite = false;
		ipcStore->deleteData(storeId);
	}
	return result;
}

ReturnValue_t TmStore::requestChunkOfData() {
	uint32_t readSize =
			(maximumAmountToRead > chunkSize()) ?
					chunkSize() : maximumAmountToRead;
	CommandMessage message;
	if (readSize > ring.readTillWrap(TEMP_READ_PTR)) {
		//Wrap
		splitReadTotalSize = readSize;
		splitRead = SPLIT_STARTED;
		MemoryMessage::setMemoryDumpCommand(&message,
				ring.getRead(TEMP_READ_PTR), ring.readTillWrap(TEMP_READ_PTR));
	} else {
		splitRead = NO_SPLIT;
		MemoryMessage::setMemoryDumpCommand(&message,
				ring.getRead(TEMP_READ_PTR), readSize);
	}
	ReturnValue_t result = memoryQueue.sendToDefault(&message);
	if (result != RETURN_OK) {
		splitRead = NO_SPLIT;
		maximumAmountToRead = 0;
	}
	return result;
}

ReturnValue_t TmStore::initialize() {
	AcceptsMemoryMessagesIF* memoryTarget = objectManager->get<
			AcceptsMemoryMessagesIF>(memoryObject);
	if (memoryTarget == NULL) {
		return RETURN_FAILED;
	}
	memoryQueue.setDefaultDestination(memoryTarget->getCommandQueue());
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (ipcStore == NULL) {
		return RETURN_FAILED;
	}
	//assert that Store is larger than local store and potential data on the way.
	if (ring.availableWriteSpace(READ_PTR) < (2 * tmBuffer.maxSize())) {
		//Ring buffer is too small.
		return RETURN_FAILED;
	}
	//Ugly cast to have the correct link to eventProxy:
	eventProxy = dynamic_cast<EventReportingProxyIF*>(owner);
	if (eventProxy == NULL) {
		return RETURN_FAILED;
	}
	return RETURN_OK;
}

ReturnValue_t TmStore::initializeStore() {
	setState(STARTUP);
	eventProxy->forwardEvent(STORE_INITIALIZE, 0, 0);
	return RETURN_OK;
}

bool TmStore::isReady() {
	return (state == READY);
}

ReturnValue_t TmStore::sendRemainingTmPiece() {
	if (writeState != SENDING_PACKETS) {
		return RETURN_FAILED;
	}
	CommandMessage message;
	MemoryMessage::setMemoryLoadCommand(&message, ring.getStart(),
			splitWriteStoreId);
	return memoryQueue.sendToDefault(&message);
}

ReturnValue_t TmStore::requestRemainingDumpPiece(uint32_t firstPartSize) {
	CommandMessage message;
	MemoryMessage::setMemoryDumpCommand(&message, ring.getStart(),
			splitReadTotalSize - firstPartSize);
	return memoryQueue.sendToDefault(&message);
}

uint32_t TmStore::availableData() {
	return ring.availableReadData();
}

ReturnValue_t TmStore::storeOrForwardPacket(const uint8_t* data,
		uint32_t size) {
	if (tmBuffer.remaining() >= size) {
		memcpy(&tmBuffer[tmBuffer.size], data, size);
		tmBuffer.size += size;
		return RETURN_OK;
	} else {
		if (writeState != WRITE_IDLE) {
			return BUSY;
		}
		if (!isReady()) {
			return NOT_READY;
		}
		sendTmBufferToStore();
		memcpy(tmBuffer.front(), data, size);
		tmBuffer.size += size;
		return RETURN_OK;
	}
}

bool TmStore::hasEnoughSpaceFor(uint32_t size) {
	//Correct size configuration is asserted in initialize().
	if ((ring.availableWriteSpace(READ_PTR) - tmBuffer.size - pendingDataToWrite)
			> size) {
		return true;
	} else {
		return false;
	}
}

ReturnValue_t TmStore::deleteContent(bool deletePart, uint32_t upToAddress,
		uint32_t nDeletedPackets, TmPacketMinimal* newOldestPacket) {
	if (deletePart) {
		ring.setRead(upToAddress, READ_PTR);
		storedPacketCounter -= nDeletedPackets;
		if (newOldestPacket != NULL) {
			oldestPacket.setContent(newOldestPacket);
		}
	} else {
		resetStore();
	}
	tryToSetStoreInfo = true;
	return RETURN_OK;
}

ReturnValue_t TmStore::handleFullStore(const uint8_t* data, uint32_t size) {
	if (!isReady()) {
		return NOT_READY;
	}
	if (!ring.overwritesOld()) {
		sendTmBufferToStore();
		return FULL;
	}
	ReturnValue_t result = FULL;
	if ((writeState == WRITE_IDLE) && (readState == READ_IDLE)) {
		sendTmBufferToStore();
		maximumAmountToRead = chunkSize();
		ring.setRead(ring.getRead(READ_PTR), TEMP_READ_PTR);
		result = requestChunkOfData();
		if (result != RETURN_OK) {
			return result;
		}
		setReadState(DELETING_OLD);
		//Store data in cleared store.
		memcpy(tmBuffer.front(), data, size);
		tmBuffer.size += size;
		result = RETURN_OK;
	} else if (readState == DELETING_OLD) {
		//Try to store local as long as possible
		if (tmBuffer.remaining() >= size) {
			memcpy(&tmBuffer[tmBuffer.size], data, size);
			tmBuffer.size += size;
			result = RETURN_OK;
		}
	}
	return result;
}

void TmStore::deleteOld(const uint8_t* buffer, uint32_t size) {
	ReturnValue_t result = RETURN_OK;
	uint32_t tempSize = 0;
	while (size >= TmPacketMinimal::MINIMUM_SIZE && result == RETURN_OK) {
		TmPacketMinimal packet(buffer);
		tempSize = packet.getFullSize();
		if (readState == DELETING_MORE) {
			if (size < (TmPacketMinimal::MINIMUM_SIZE + tempSize)) {
				oldestPacket.setContent(&packet);
				break;
			}
		} else {
			if (tempSize > size) {
				//Don't delete the last packet half in store...
				break;
			}
		}
		result = ring.readData(tempSize, READ_PTR);
		ring.readData(tempSize, TEMP_READ_PTR);
		size -= tempSize;
		buffer += tempSize;
		storedPacketCounter--;
	}
	if (readState == DELETING_OLD) {
		result = requestChunkOfData();
		if (result != HasReturnvaluesIF::RETURN_OK) {
			oldestPacket.reset();
			eventProxy->forwardEvent(STORE_SEND_WRITE_FAILED, result, 1);
			setReadState(READ_IDLE);
		} else {
			setReadState(DELETING_MORE);
		}
	} else {
		setReadState(READ_IDLE);
		owner->restDownlinkedPacketCount();
		tryToSetStoreInfo = true;
	}
}

void TmStore::sendTmBufferToStore() {
	//No need to send empty data packet.
	if (tmBuffer.size > 0) {
		ReturnValue_t result = sendDataToTmStore(tmBuffer.front(), tmBuffer.size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			eventProxy->forwardEvent(STORE_SEND_WRITE_FAILED, result, 2);
		}
		tmBuffer.clear();
	}
}

void TmStore::resetStore(bool resetWrite, bool resetRead) {
	ring.clear();
	setState(READY);
	storedPacketCounter = 0;
	clearPending();
	tmBuffer.clear();
	oldestPacket.reset();
	newestPacket.reset();
	tryToSetStoreInfo = true;
	if (resetWrite) {
		setWriteState(WRITE_IDLE);
		splitWrite = false;
		pendingDataToWrite = 0;
	}
	if (resetRead) {
		maximumAmountToRead = 0;
		splitRead = NO_SPLIT;
		setReadState(READ_IDLE);
	}
}

void TmStore::setState(State state) {
//	debug << "TmStore::setState: " << state << std::endl;
	this->state = state;
}

void TmStore::setWriteState(WriteState writeState) {
//	debug << "TmStore::setWriteState: " << writeState << std::endl;
	this->writeState = writeState;
}

void TmStore::setReadState(ReadState readState) {
//	debug << "TmStore::setReadState: " << readState << std::endl;
	this->readState = readState;
}

float TmStore::getPercentageFilled() const {
	return ring.availableReadData(READ_PTR) / float(ring.maxSize());
}

uint32_t TmStore::getStoredPacketsCount() const {
	return storedPacketCounter;
}

TmPacketInformation* TmStore::getOldestPacket() {
	return &oldestPacket;
}

TmPacketInformation* TmStore::getYoungestPacket() {
	return &newestPacket;
}

uint32_t TmStore::chunkSize() {
	return tmBuffer.maxSize();
}

void TmStore::clearPending() {
	pendingOldestPacket.reset();
	pendingNewestPacket.reset();
	pendingStoredPackets = 0;
}
