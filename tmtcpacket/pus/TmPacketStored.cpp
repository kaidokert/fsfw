#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TmPacketStored.h>
#include <framework/tmtcservices/TmTcMessage.h>
#include <string.h>

TmPacketStored::TmPacketStored(store_address_t setAddress) :
		TmPacketBase(NULL), storeAddress(setAddress) {
	setStoreAddress(storeAddress);
}

TmPacketStored::TmPacketStored(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packetSubcounter, const uint8_t* data,
		uint32_t size, const uint8_t* headerData, uint32_t headerSize) :
		TmPacketBase(NULL) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (!checkAndSetStore()) {
		return;
	}
	uint8_t* pData = NULL;
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
		uint8_t subservice, uint8_t packetSubcounter, SerializeIF* content,
		SerializeIF* header) :
		TmPacketBase(NULL) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if (!checkAndSetStore()) {
		return;
	}
	uint32_t sourceDataSize = 0;
	if (content != NULL) {
		sourceDataSize += content->getSerializedSize();
	}
	if (header != NULL) {
		sourceDataSize += header->getSerializedSize();
	}
	uint8_t* p_data = NULL;
	ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
			(TmPacketBase::TM_PACKET_MIN_SIZE + sourceDataSize), &p_data);
	if (returnValue != store->RETURN_OK) {
		checkAndReportLostTm();
	}
	setData(p_data);
	initializeTmPacket(apid, service, subservice, packetSubcounter);
	uint8_t* putDataHere = getSourceData();
	uint32_t size = 0;
	if (header != NULL) {
		header->serialize(&putDataHere, &size, sourceDataSize, true);
	}
	if (content != NULL) {
		content->serialize(&putDataHere, &size, sourceDataSize, true);
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
	setData(NULL);
}

void TmPacketStored::setStoreAddress(store_address_t setAddress) {
	storeAddress = setAddress;
	const uint8_t* temp_data = NULL;
	size_t temp_size;
	if (!checkAndSetStore()) {
		return;
	}
	ReturnValue_t status = store->getData(storeAddress, &temp_data, &temp_size);
	if (status == StorageManagerIF::RETURN_OK) {
		setData(temp_data);
	} else {
		setData(NULL);
		storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	}
}

bool TmPacketStored::checkAndSetStore() {
	if (store == NULL) {
		store = objectManager->get<StorageManagerIF>(objects::TM_STORE);
		if (store == NULL) {
			sif::error << "TmPacketStored::TmPacketStored: TM Store not found!"
					<< std::endl;
			return false;
		}
	}
	return true;
}

StorageManagerIF* TmPacketStored::store = NULL;
InternalErrorReporterIF* TmPacketStored::internalErrorReporter = NULL;

ReturnValue_t TmPacketStored::sendPacket(MessageQueueId_t destination,
		MessageQueueId_t sentFrom, bool doErrorReporting) {
	if (getWholeData() == NULL) {
		//SHOULDDO: More decent code.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	TmTcMessage tmMessage(getStoreAddress());
	ReturnValue_t result = MessageQueueSenderIF::sendMessage(destination, &tmMessage, sentFrom);
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
	if (internalErrorReporter == NULL) {
		internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
				objects::INTERNAL_ERROR_REPORTER);
	}
	if (internalErrorReporter != NULL) {
		internalErrorReporter->lostTm();
	}
}
