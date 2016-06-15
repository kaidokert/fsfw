/*
 * TmStoreInfo.h
 *
 *  Created on: 19.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_TMSTORAGE_TMSTOREINFO_H_
#define FRAMEWORK_TMSTORAGE_TMSTOREINFO_H_

#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>

class TmStoreInfo: public SerializeIF {
public:
	SerializeElement<uint32_t> readP;
	SerializeElement<uint32_t> writeP;
	SerializeElement<uint32_t> storedPacketCount;
	LinkedElement<SerializeIF> linkedOldestPacket;
	LinkedElement<SerializeIF> linkedYoungestPacket;
	SerializeElement<uint16_t> crc;
	TmStoreInfo(SerializeIF* oldestPacket, SerializeIF* youngestPacket) :
			readP(0), writeP(0), linkedOldestPacket(oldestPacket), linkedYoungestPacket(
					youngestPacket) {
		this->readP.setNext(&this->writeP);
		this->writeP.setNext(&this->storedPacketCount);
		storedPacketCount.setNext(&linkedOldestPacket);
		linkedOldestPacket.setNext(&linkedYoungestPacket);
	}
	void setContent(uint32_t readP, uint32_t writeP, uint32_t storedPackets) {
		this->readP = readP;
		this->writeP = writeP;
		this->storedPacketCount = storedPackets;
	}
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		uint8_t* startBuffer = *buffer;
		ReturnValue_t result = SerialLinkedListAdapter<SerializeIF>::serialize(
				&readP, buffer, size, max_size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		uint32_t elementSize =
				SerialLinkedListAdapter<SerializeIF>::getSerializedSize(&readP);
		SerializeElement<uint16_t> localCrc;
		localCrc.entry = ::Calculate_CRC(startBuffer, elementSize);
		return localCrc.serialize(buffer, size, max_size, bigEndian);
	}

	uint32_t getSerializedSize() const {
		return (SerialLinkedListAdapter<SerializeIF>::getSerializedSize(&readP)
				+ crc.getSerializedSize());
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
	bool bigEndian) {
		ReturnValue_t result =
				SerialLinkedListAdapter<SerializeIF>::deSerialize(&readP,
						buffer, size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		return crc.deSerialize(buffer, size, bigEndian);
	}
};

#endif /* FRAMEWORK_TMSTORAGE_TMSTOREINFO_H_ */
