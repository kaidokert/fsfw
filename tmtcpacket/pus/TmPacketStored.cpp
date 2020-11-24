#include "TmPacketStored.h"

#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tmtcservices/TmTcMessage.h"

#include <cstring>

StorageManagerIF *TmPacketStored::store = nullptr;
InternalErrorReporterIF *TmPacketStored::internalErrorReporter = nullptr;

TmPacketStored::TmPacketStored(store_address_t setAddress) :
		TmPacketBase(nullptr), storeAddress(setAddress) {
	setStoreAddress(storeAddress);
}

TmPacketStored::TmPacketStored(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packetSubcounter, const uint8_t *data,
		uint32_t size, const uint8_t *headerData, uint32_t headerSize) :
		TmPacketBase(NULL) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (not checkAndSetStore()) {
		return;
	}
	uint8_t *pData = nullptr;
	ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
			(TmPacketBase::TM_PACKET_MIN_SIZE + size + headerSize), &pData);

	if (returnValue != store->RETURN_OK) {
		checkAndReportLostTm();
		return;
	}
	setData(pData);
	initializeTmPacket(apid, service, subservice, packetSubcounter);
	memcpy(getSourceData(), headerData, headerSize);
	memcpy(getSourceData() + headerSize, data, size);
	setPacketDataLength(
			size + headerSize + sizeof(PUSTmDataFieldHeader) + CRC_SIZE - 1);
}

TmPacketStored::TmPacketStored(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packetSubcounter, SerializeIF *content,
		SerializeIF *header) :
		TmPacketBase(NULL) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (not checkAndSetStore()) {
		return;
	}
	size_t sourceDataSize = 0;
	if (content != NULL) {
		sourceDataSize += content->getSerializedSize();
	}
	if (header != NULL) {
		sourceDataSize += header->getSerializedSize();
	}
	uint8_t *p_data = NULL;
	ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
			(TmPacketBase::TM_PACKET_MIN_SIZE + sourceDataSize), &p_data);
	if (returnValue != store->RETURN_OK) {
		checkAndReportLostTm();
	}
	setData(p_data);
	initializeTmPacket(apid, service, subservice, packetSubcounter);
	uint8_t *putDataHere = getSourceData();
	size_t size = 0;
	if (header != NULL) {
		header->serialize(&putDataHere, &size, sourceDataSize,
				SerializeIF::Endianness::BIG);
	}
	if (content != NULL) {
		content->serialize(&putDataHere, &size, sourceDataSize,
				SerializeIF::Endianness::BIG);
	}
	setPacketDataLength(
			sourceDataSize + sizeof(PUSTmDataFieldHeader) + CRC_SIZE - 1);
}

store_address_t TmPacketStored::getStoreAddress() {
	return storeAddress;
}

void TmPacketStored::deletePacket() {
	store->deleteData(storeAddress);
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	setData(nullptr);
}

void TmPacketStored::setStoreAddress(store_address_t setAddress) {
	storeAddress = setAddress;
	const uint8_t* tempData = nullptr;
	size_t tempSize;
	if (not checkAndSetStore()) {
		return;
	}
	ReturnValue_t status = store->getData(storeAddress, &tempData, &tempSize);
	if (status == StorageManagerIF::RETURN_OK) {
		setData(tempData);
	} else {
		setData(nullptr);
		storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	}
}

bool TmPacketStored::checkAndSetStore() {
	if (store == nullptr) {
		store = objectManager->get<StorageManagerIF>(objects::TM_STORE);
		if (store == nullptr) {
			sif::error << "TmPacketStored::TmPacketStored: TM Store not found!"
					<< std::endl;
			return false;
		}
	}
	return true;
}

ReturnValue_t TmPacketStored::sendPacket(MessageQueueId_t destination,
		MessageQueueId_t sentFrom, bool doErrorReporting) {
	if (getWholeData() == nullptr) {
		//SHOULDDO: More decent code.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	TmTcMessage tmMessage(getStoreAddress());
	ReturnValue_t result = MessageQueueSenderIF::sendMessage(destination,
			&tmMessage, sentFrom);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		deletePacket();
		if (doErrorReporting) {
			checkAndReportLostTm();
		}
		return result;
	}
	//SHOULDDO: In many cases, some counter is incremented for successfully sent packets. The check is often not done, but just incremented.
	return HasReturnvaluesIF::RETURN_OK;

}

void TmPacketStored::checkAndReportLostTm() {
	if (internalErrorReporter == nullptr) {
		internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
				objects::INTERNAL_ERROR_REPORTER);
	}
	if (internalErrorReporter != nullptr) {
		internalErrorReporter->lostTm();
	}
}
