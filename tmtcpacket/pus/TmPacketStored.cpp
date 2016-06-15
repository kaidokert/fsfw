/*
 * TmPacketStored.cpp
 *
 *  Created on: 19.11.2012
 *      Author: baetz
 */

#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcpacket/pus/TmPacket.h>
#include <framework/tmtcpacket/pus/TmPacketStored.h>
#include <string.h>

TmPacketStored::TmPacketStored( store_address_t setAddress  ) : TmPacketBase(NULL), storeAddress(setAddress) {
	this->setStoreAddress( this->storeAddress );
}

TmPacketStored::TmPacketStored( uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packet_counter, const uint8_t* data, uint32_t size,const uint8_t* headerData, uint32_t headerSize) : TmPacketBase(NULL) {
	this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if ( this->checkAndSetStore() ) {
		TmPacket<TmPacketBase::TM_PACKET_MIN_SIZE> temp_packet( apid, service, subservice, packet_counter );
		uint8_t* p_data = NULL;
		ReturnValue_t returnValue = this->store->getFreeElement( &this->storeAddress, (TmPacketBase::TM_PACKET_MIN_SIZE + size + headerSize), &p_data );
		if ( returnValue == this->store->RETURN_OK ) {
			memcpy(p_data, temp_packet.getWholeData(), temp_packet.getFullSize() );
			this->setData( p_data );
			memcpy( this->getSourceData(), headerData, headerSize );
			memcpy( this->getSourceData() + headerSize, data, size );
			this->setPacketDataLength( size + headerSize + sizeof(PUSTmDataFieldHeader) + CRC_SIZE  - 1 );
			this->setErrorControl();
		}
	}
}

TmPacketStored::TmPacketStored(uint16_t apid, uint8_t service,
		uint8_t subservice, uint8_t packet_counter, SerializeIF* content,
		SerializeIF* header) : TmPacketBase(NULL) {
	storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	if ( checkAndSetStore() ) {
		TmPacket<TmPacketBase::TM_PACKET_MIN_SIZE> temp_packet( apid, service, subservice, packet_counter );
		uint32_t sourceDataSize = 0;
		if (content != NULL) {
			sourceDataSize += content->getSerializedSize();
		}
		if (header != NULL) {
			sourceDataSize += header->getSerializedSize();
		}
		uint8_t* p_data = NULL;
		ReturnValue_t returnValue = this->store->getFreeElement( &this->storeAddress, (TmPacketBase::TM_PACKET_MIN_SIZE + sourceDataSize), &p_data );
		if ( returnValue == store->RETURN_OK ) {
			memcpy(p_data, temp_packet.getWholeData(), temp_packet.getFullSize() );
			this->setData( p_data );
			uint8_t* putDataHere = getSourceData();
			uint32_t size = 0;
			if (header != NULL) {
				header->serialize(&putDataHere, &size, sourceDataSize, true);
			}
			if (content != NULL) {
				content->serialize(&putDataHere, &size, sourceDataSize, true);
			}
			this->setPacketDataLength( sourceDataSize + sizeof(PUSTmDataFieldHeader) + CRC_SIZE  - 1 );
			this->setErrorControl();
		}
	}
}

store_address_t TmPacketStored::getStoreAddress() {
	return this->storeAddress;
}

void TmPacketStored::deletePacket() {
	this->store->deleteData( this->storeAddress );
	this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	this->setData( NULL );
}

void TmPacketStored::setStoreAddress(store_address_t setAddress) {
	this->storeAddress = setAddress;
	const uint8_t* temp_data = NULL;
	uint32_t temp_size;
	if (!checkAndSetStore()) {
		return;
	}
	ReturnValue_t status = this->store->getData(this->storeAddress, &temp_data, &temp_size );
	if (status == StorageManagerIF::RETURN_OK) {
		this->setData(temp_data);
	} else {
		this->setData(NULL);
		this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
	}
}

bool TmPacketStored::checkAndSetStore() {
	if (this->store == NULL) {
		this->store = objectManager->get<StorageManagerIF>( objects::TM_STORE );
		if ( this->store == NULL ) {
			error << "TmPacketStored::TmPacketStored: TM Store not found!" << std::endl;
			return false;
		}
	}
	return true;
}

StorageManagerIF* TmPacketStored::store = NULL;
