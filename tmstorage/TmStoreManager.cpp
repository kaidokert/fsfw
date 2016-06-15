/*
 * TmStoreManager.cpp
 *
 *  Created on: 18.02.2015
 *      Author: baetz
 */

#include <framework/tmstorage/TmStoreManager.h>
#include <framework/tmstorage/TmStoreMessage.h>
#include <framework/tmtcpacket/SpacePacketBase.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tmtcservices/TmTcMessage.h>

TmStoreManager::TmStoreManager(object_id_t objectId, object_id_t setDumpTarget,
		uint8_t setVC, uint32_t setTimeoutMs) :
		SystemObject(objectId), backend(NULL), tmForwardStore(NULL), dumpTarget(
				setDumpTarget), virtualChannel(setVC), fetchState(
				NOTHING_FETCHED), addressOfFetchCandidate(0), deletionStarted(
		false), lastAddressToDelete(0), pendingPacketsToDelete(0), state(IDLE), storingEnabled(
		false), timeoutMs(setTimeoutMs), ipcStore(NULL), downlinkedPacketsCount(
				0), fullEventThrown(false) {
}

TmStoreManager::~TmStoreManager() {
}

ReturnValue_t TmStoreManager::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}
	AcceptsTelemetryIF* telemetryDestimation = objectManager->get<
			AcceptsTelemetryIF>(dumpTarget);
	if (telemetryDestimation == NULL) {
		return RETURN_FAILED;
	}
	tmQueue.setDefaultDestination(
			telemetryDestimation->getReportReceptionQueue(virtualChannel));
	tmForwardStore = objectManager->get<StorageManagerIF>(objects::TM_STORE);
	if (tmForwardStore == NULL) {
		return RETURN_FAILED;
	}
	result = backend->initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (ipcStore == NULL) {
		return RETURN_FAILED;
	}
	return matcher.initialize();
}

ReturnValue_t TmStoreManager::fetchPackets(ApidSsc start, ApidSsc end) {
	//Check mode, store ids, start fetching.
	if (state != IDLE) {
		return BUSY;
	}
	if (start.apid < SpacePacketBase::LIMIT_APID) {
		firstPacketToFetch = start;
		fetchCandidate.apid = start.apid;
		fetchState = NOTHING_FETCHED;
	} else {
		firstPacketToFetch.apid = SpacePacketBase::LIMIT_APID;
		firstPacketToFetch.ssc = 0;
		//There's no start defined, so we're in range automatically.
		fetchState = IN_RANGE;
	}
	if (end.apid < SpacePacketBase::LIMIT_APID) {
		lastPacketToFetch = end;
	} else {
		lastPacketToFetch.apid = SpacePacketBase::LIMIT_APID;
		lastPacketToFetch.ssc = SpacePacketBase::LIMIT_SEQUENCE_COUNT;
	}
	//Advanced: start fetching at a certain position
	ReturnValue_t result = backend->fetchPackets();
	if (result != RETURN_OK) {
		return result;
	}
	downlinkedPacketsCount = 0;
	state = RETRIEVING_PACKETS;
	return result;
}

ReturnValue_t TmStoreManager::performOperation() {
	backend->performOperation();
	checkCommandQueue();
	return RETURN_OK;
}

ReturnValue_t TmStoreManager::packetRetrieved(TmPacketMinimal* packet,
		uint32_t address, bool isLastPacket) {
	ReturnValue_t result = RETURN_FAILED;
	switch (state) {
	case DELETING_PACKETS:
		result = checkDeletionLimit(packet, address);
		break;
	case RETRIEVING_PACKETS:
		result = checkRetrievalLimit(packet, address);
		break;
	case GET_OLDEST_PACKET_INFO: {
		backend->deleteContent(true, lastAddressToDelete,
				pendingPacketsToDelete, packet);
		result = LAST_PACKET_FOUND;
	}
		break;
	default:
		triggerEvent(TmStoreBackendIF::UNEXPECTED_MSG, 0, packet->getPacketSequenceCount());
		break;
	}
	switch (result) {
	case RETURN_OK:
		//We go on. But if lastPacket...
		if (isLastPacket) {
			//... we'll stop successfully.
			replySuccess();
			if (state == DELETING_PACKETS) {
				//Store is completely deleted, so we can reset oldest and newest packet.
				restDownlinkedPacketCount();
				backend->resetStore();
			}
			state = IDLE;
		}
		break;
	case LAST_PACKET_FOUND:
		//All ok
		replySuccess();
		state = IDLE;
		break;
	case STOP_FETCH:
		//This means, we started a new fetch. Returning STOP_FETCH will stop current scan. Stay in state.
		break;
	default:
		//Some error occurred.
		replyFailure(result);
		state = IDLE;
		break;
	}
	return result;
}

ReturnValue_t TmStoreManager::checkRetrievalLimit(TmPacketMinimal* packet,
		uint32_t address) {
	switch (fetchState) {
	case NOTHING_FETCHED:
	case BEFORE_RANGE:
		if (packet->getAPID() != firstPacketToFetch.apid) {
			//Dump all packets of unknown APID.
			dumpPacket(packet);
			return RETURN_OK;
		}
		if (packet->getPacketSequenceCount() < firstPacketToFetch.ssc) {
			addressOfFetchCandidate = address;
			fetchCandidate.ssc = packet->getPacketSequenceCount();
			fetchState = BEFORE_RANGE;
			return RETURN_OK;
		} else if ((packet->getPacketSequenceCount() == firstPacketToFetch.ssc)
				|| (fetchState == NOTHING_FETCHED)) {
			//We have either found the right packet or one with higher count without having an older packet. So dump.
			fetchState = IN_RANGE;
			dumpPacket(packet);
			if (packet->getPacketSequenceCount() < lastPacketToFetch.ssc) {
				return RETURN_OK;
			} else {
				return LAST_PACKET_FOUND;
			}
		} else {
			//We're in range, but the expected SSC is not in store. So we shall restart from the first older packet.
			ReturnValue_t result = backend->fetchPackets(true,
					addressOfFetchCandidate);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
			fetchState = NOTHING_FETCHED;
			firstPacketToFetch = fetchCandidate;
			return STOP_FETCH;
		}
	case IN_RANGE:
		dumpPacket(packet);
		if ((packet->getAPID() == lastPacketToFetch.apid)
				&& (packet->getPacketSequenceCount() >= lastPacketToFetch.ssc)) {
			return LAST_PACKET_FOUND;
		} else {
			return RETURN_OK;
		}
	default:
		break;
	}
	return RETURN_FAILED;
}

ReturnValue_t TmStoreManager::deletePackets(ApidSsc upTo) {
	if (state != IDLE) {
		return BUSY;
	}
	if (upTo.apid >= SpacePacketBase::LIMIT_APID) {
		ReturnValue_t result = backend->deleteContent();
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		restDownlinkedPacketCount();
		return result;
	} else {
		state = DELETING_PACKETS;
		restDownlinkedPacketCount();
		lastPacketToDelete = upTo;
		deletionStarted = false;
		pendingPacketsToDelete = 0;
		return backend->fetchPackets();
	}
}

ReturnValue_t TmStoreManager::checkDeletionLimit(TmPacketMinimal* packet,
		uint32_t address) {
	uint32_t addressOfLastByte = address + packet->getFullSize();
	if (packet->getAPID() == lastPacketToDelete.apid) {
		if (packet->getPacketSequenceCount() < lastPacketToDelete.ssc) {
			if (deletionStarted) {
				backend->deleteContent(true, lastAddressToDelete,
						pendingPacketsToDelete, packet);
				pendingPacketsToDelete = 0;
			} else {
				deletionStarted = true;
			}
			pendingPacketsToDelete++;
			lastAddressToDelete = addressOfLastByte;
			return RETURN_OK;
		} else if (packet->getPacketSequenceCount() == lastPacketToDelete.ssc) {
			lastAddressToDelete = addressOfLastByte;
			pendingPacketsToDelete++;
			state = GET_OLDEST_PACKET_INFO;
			return RETURN_OK;
		} else {
			return LAST_PACKET_FOUND;
		}
	} else {
		lastAddressToDelete = addressOfLastByte;
		pendingPacketsToDelete++;
		return RETURN_OK;
	}
}

void TmStoreManager::setBackend(TmStoreBackendIF* backend) {
	this->backend = backend;
}

TmStoreBackendIF* TmStoreManager::getBackend() const {
	return backend;
}

ReturnValue_t TmStoreManager::checkPacket(SpacePacketBase* tmPacket) {
	if (!storingEnabled) {
		return RETURN_OK;
	}
	TmPacketMinimal testPacket(tmPacket->getWholeData());
	if (matcher.match(&testPacket)) {
		ReturnValue_t result = backend->storePacket(&testPacket);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			//Generate only one event to avoid full event store loop.
			if (!fullEventThrown) {
				triggerEvent(TmStoreBackendIF::STORING_FAILED, result, testPacket.getPacketSequenceCount());
				fullEventThrown = true;
			}
		} else {
			fullEventThrown = false;
		}
		return result;
	} else {
		return RETURN_OK;
	}
}

void TmStoreManager::dumpPacket(SpacePacketBase* packet) {
	store_address_t forwardStoreId;
	TmTcMessage message;
	ReturnValue_t result = tmForwardStore->addData(&forwardStoreId,
			packet->getWholeData(), packet->getFullSize());
	if (result != RETURN_OK) {
		triggerEvent(TmStoreBackendIF::TM_DUMP_FAILED, result, packet->getPacketSequenceCount());
		return;
	}
	message.setStorageId(forwardStoreId);
	result = tmQueue.sendToDefault(&message);
	if (result != RETURN_OK) {
		triggerEvent(TmStoreBackendIF::TM_DUMP_FAILED, result, packet->getPacketSequenceCount());
		tmForwardStore->deleteData(forwardStoreId);
		return;
	}
	downlinkedPacketsCount.entry++;
}

bool TmStoreManager::isEnabled() const {
	return storingEnabled;
}

void TmStoreManager::setEnabled(bool enabled) {
	storingEnabled = enabled;
}

void TmStoreManager::checkCommandQueue() {
	if (state != IDLE) {
		if (timer.hasTimedOut()) {
			replyFailure(TIMEOUT);
			state = IDLE;
		}
		return;
	}
	CommandMessage message;
	for (ReturnValue_t result = commandQueue.receiveMessage(&message);
			result == RETURN_OK;
			result = commandQueue.receiveMessage(&message)) {
		switch (message.getCommand()) {
		case TmStoreMessage::ENABLE_STORING:
			setEnabled(TmStoreMessage::getEnableStoring(&message));
			replySuccess();
			break;
		case TmStoreMessage::CHANGE_SELECTION_DEFINITION: {
			uint32_t errorCount = 0;
			result = changeSelectionDefinition(
					TmStoreMessage::getAddToSelection(&message),
					TmStoreMessage::getStoreId(&message), &errorCount);
			if (result != RETURN_OK) {
				replyFailure(result, errorCount);
			} else {
				replySuccess();
			}
			break;
		}
		case TmStoreMessage::DOWNLINK_STORE_CONTENT:
			result = fetchPackets(TmStoreMessage::getPacketId1(&message),
					TmStoreMessage::getPacketId2(&message));
			if (result != RETURN_OK) {
				if (result == TmStoreBackendIF::EMPTY) {
					replySuccess();
				} else {
					replyFailure(result);
				}
			} else {
				timer.setTimeout(timeoutMs);
			}
			break;
		case TmStoreMessage::DELETE_STORE_CONTENT:
			result = deletePackets(TmStoreMessage::getPacketId1(&message));
			if (result == RETURN_OK) {
				if (state == IDLE) {
					//We're finished
					replySuccess();
				} else {
					timer.setTimeout(timeoutMs);
				}
			} else {
				replyFailure(result);
			}
			break;
		case TmStoreMessage::REPORT_SELECTION_DEFINITION:
			result = sendMatchTree();
			if (result != RETURN_OK) {
				replyFailure(result);
			}
			break;
		case TmStoreMessage::REPORT_STORE_CATALOGUE:
			result = sendStatistics();
			if (result != RETURN_OK) {
				replyFailure(result);
			}
			break;
		default:
			replyFailure(CommandMessage::UNKNOW_COMMAND, message.getCommand());
			break;
		}
	}
}

void TmStoreManager::replySuccess() {
	CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, 0);
	commandQueue.reply(&reply);

}

MessageQueueId_t TmStoreManager::getCommandQueue() {
	return commandQueue.getId();
}

void TmStoreManager::replyFailure(ReturnValue_t errorCode, uint32_t parameter) {
	CommandMessage reply(CommandMessage::REPLY_REJECTED, errorCode, parameter);
	commandQueue.reply(&reply);
}

ReturnValue_t TmStoreManager::changeSelectionDefinition(bool addSelection,
		store_address_t storeId, uint32_t* errorCount) {
	const uint8_t* pData;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(storeId, &pData, &size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	int32_t iSize = size;
	ChangeSelectionDefinition content;
	result = content.deSerialize(&pData, &iSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	ReturnValue_t totalResult = RETURN_OK;
	if (content.serviceList.size == 0) {
		return updateMatch(addSelection, content.apid, NO_SERVICE,
				NO_SUBSERVICE);
	}
	for (auto iter = content.serviceList.begin();
			iter != content.serviceList.end(); iter++) {
		if (iter->subservices.size == 0) {
			result = updateMatch(addSelection, content.apid,
					iter->service, NO_SUBSERVICE);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				totalResult = result;
				(*errorCount)++;
			}
		} else {
			for (auto iter2 = iter->subservices.begin();
					iter2 != iter->subservices.end(); iter2++) {
				result = updateMatch(addSelection, content.apid,
						iter->service, *(iter2.value));
				if (result != HasReturnvaluesIF::RETURN_OK) {
					totalResult = result;
					(*errorCount)++;
				}
			}
		}
	}
	return totalResult;
}

ReturnValue_t TmStoreManager::sendMatchTree() {
	store_address_t storeId;
	uint8_t* buffer;
	uint32_t maxSize = matcher.getSerializedSize();
	ReturnValue_t result = ipcStore->getFreeElement(&storeId, maxSize, &buffer);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	uint32_t size = 0;
	result = matcher.serialize(&buffer, &size, maxSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		ipcStore->deleteData(storeId);
		return result;
	}
	CommandMessage reply;
	TmStoreMessage::setSelectionDefinitionReportMessage(&reply, storeId);
	result = commandQueue.reply(&reply);
	if (result != RETURN_OK) {
		ipcStore->deleteData(storeId);
	}
	return result;
}

void TmStoreManager::restDownlinkedPacketCount() {
	downlinkedPacketsCount = 0;
}

ReturnValue_t TmStoreManager::updateMatch(bool addSelection, uint16_t apid, uint8_t serviceType,
		uint8_t serviceSubtype) {
	return matcher.changeMatch(addSelection, apid, serviceType, serviceSubtype);
}

void TmStoreManager::handleRetrievalFailed(ReturnValue_t errorCode,
		uint32_t parameter1, uint32_t parameter2) {
	restDownlinkedPacketCount();
	state = IDLE;
	replyFailure(errorCode, parameter1);
}

ReturnValue_t TmStoreManager::sendStatistics() {
	LinkedElement<SerializeIF> linkedOldestPacket(backend->getOldestPacket());
	LinkedElement<SerializeIF> linkedYoungestPacket(
			backend->getYoungestPacket());
	SerializeElement<float> percentageFilled(backend->getPercentageFilled());
	float pDownlinked =
			(backend->getStoredPacketsCount() != 0) ?
					(float) downlinkedPacketsCount
							/ (float) backend->getStoredPacketsCount() :
					0.0;
	SerializeElement<float> percentageDownlinked(pDownlinked);
	SerializeElement<uint32_t> storedPacketsCount(
			backend->getStoredPacketsCount());
	linkedOldestPacket.setNext(&linkedYoungestPacket);
	linkedYoungestPacket.setNext(&percentageFilled);
	percentageFilled.setNext(&percentageDownlinked);
	percentageDownlinked.setNext(&storedPacketsCount);
	storedPacketsCount.setNext(&downlinkedPacketsCount);
	store_address_t storeId;
	uint8_t* buffer;
	uint32_t maxSize = SerialLinkedListAdapter<SerializeIF>::getSerializedSize(
			&linkedOldestPacket);
	ReturnValue_t result = ipcStore->getFreeElement(&storeId, maxSize, &buffer);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	uint32_t size = 0;
	result = SerialLinkedListAdapter<SerializeIF>::serialize(
			&linkedOldestPacket, &buffer, &size, maxSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		ipcStore->deleteData(storeId);
		return result;
	}
	CommandMessage reply;
	TmStoreMessage::setStoreCatalogueReportMessage(&reply, storeId);
	result = commandQueue.reply(&reply);
	if (result != RETURN_OK) {
		ipcStore->deleteData(storeId);
	}
	return result;
}
