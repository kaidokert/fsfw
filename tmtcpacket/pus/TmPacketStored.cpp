#include "TmPacketStored.h"

#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tmtcservices/TmTcMessage.h"

#include <cstring>

StorageManagerIF *TmPacketStoredPusA::store = nullptr;
InternalErrorReporterIF *TmPacketStoredPusA::internalErrorReporter = nullptr;

TmPacketStoredPusA::TmPacketStoredPusA(store_address_t setAddress) :
		TmPacketStoredBase(setAddress), TmPacketPusA(nullptr){
}

TmPacketStoredPusA::TmPacketStoredPusA(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packetSubcounter, const uint8_t *data,
		uint32_t size, const uint8_t *headerData, uint32_t headerSize) :
		TmPacketPusA(nullptr) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (not TmPacketStoredBase::checkAndSetStore()) {
		return;
	}
	uint8_t *pData = nullptr;
	ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
			(getPacketMinimumSize() + size + headerSize), &pData);

	if (returnValue != store->RETURN_OK) {
	    TmPacketStoredBase::checkAndReportLostTm();
		return;
	}
	setData(pData);
	initializeTmPacket(apid, service, subservice, packetSubcounter);
	memcpy(getSourceData(), headerData, headerSize);
	memcpy(getSourceData() + headerSize, data, size);
	setPacketDataLength(
			size + headerSize + sizeof(PUSTmDataFieldHeaderPusA) + CRC_SIZE - 1);
}

TmPacketStoredPusA::TmPacketStoredPusA(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packetSubcounter, SerializeIF *content,
		SerializeIF *header) :
		TmPacketPusA(nullptr) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (not  TmPacketStoredBase::checkAndSetStore()) {
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
			(getPacketMinimumSize() + sourceDataSize), &p_data);
	if (returnValue != store->RETURN_OK) {
	    TmPacketStoredBase::checkAndReportLostTm();
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
			sourceDataSize + sizeof(PUSTmDataFieldHeaderPusA) + CRC_SIZE - 1);
}

uint8_t* TmPacketStoredPusA::getAllTmData() {
    return getWholeData();
}

void TmPacketStoredPusA::setDataPointer(const uint8_t *newPointer) {
    setData(newPointer);
}
