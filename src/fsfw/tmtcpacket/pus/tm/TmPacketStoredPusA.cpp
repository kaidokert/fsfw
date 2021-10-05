#include "fsfw/tmtcpacket/pus/tm/TmPacketStoredPusA.h"

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

#include <cstring>

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
	if (not TmPacketStoredBase::checkAndSetStore()) {
		return;
	}
	size_t sourceDataSize = 0;
	if (content != nullptr) {
		sourceDataSize += content->getSerializedSize();
	}
	if (header != nullptr) {
		sourceDataSize += header->getSerializedSize();
	}
	uint8_t *pData = nullptr;
	size_t sizeToReserve = getPacketMinimumSize() + sourceDataSize;
	ReturnValue_t returnValue = store->getFreeElement(&storeAddress,
			sizeToReserve, &pData);
	if (returnValue != store->RETURN_OK) {
	    TmPacketStoredBase::checkAndReportLostTm();
#if FSFW_VERBOSE_LEVEL >= 1
        switch(returnValue) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        case(StorageManagerIF::DATA_STORAGE_FULL): {
            sif::warning << "TmPacketStoredPusA::TmPacketStoredPusC: Store full for packet with "
                    "size " << sizeToReserve << std::endl;
            break;
        }
        case(StorageManagerIF::DATA_TOO_LARGE): {
            sif::warning << "TmPacketStoredPusA::TmPacketStoredPusC: Data with size " <<
                    sizeToReserve << " too large" <<  std::endl;
            break;
        }
#else
        case(StorageManagerIF::DATA_STORAGE_FULL): {
            sif::printWarning("TmPacketStoredPusA::TmPacketStoredPusC: Store full for packet with "
                    "size %d\n", sizeToReserve);
            break;
        }
        case(StorageManagerIF::DATA_TOO_LARGE): {
            sif::printWarning("TmPacketStoredPusA::TmPacketStoredPusC: Data with size "
                    "%d too large\n", sizeToReserve);
            break;
        }
#endif
#endif
	    }
	    return;
	}
	setData(pData);
	initializeTmPacket(apid, service, subservice, packetSubcounter);
	uint8_t *putDataHere = getSourceData();
	size_t size = 0;
	if (header != nullptr) {
		header->serialize(&putDataHere, &size, sourceDataSize,
				SerializeIF::Endianness::BIG);
	}
	if (content != nullptr) {
		content->serialize(&putDataHere, &size, sourceDataSize,
				SerializeIF::Endianness::BIG);
	}
	setPacketDataLength(sourceDataSize + sizeof(PUSTmDataFieldHeaderPusA) + CRC_SIZE - 1);
}

uint8_t* TmPacketStoredPusA::getAllTmData() {
    return getWholeData();
}

void TmPacketStoredPusA::setDataPointer(const uint8_t *newPointer) {
    setData(newPointer);
}
