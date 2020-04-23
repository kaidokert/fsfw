#include <framework/datapool/DataPool.h>
#include <framework/datapool/PoolEntryIF.h>
#include <framework/datapool/PoolRawAccess.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/osal/Endiness.h>

PoolRawAccess::PoolRawAccess(uint32_t set_id, uint8_t setArrayEntry,
		DataSetIF* data_set, ReadWriteMode_t setReadWriteMode) :
		dataPoolId(set_id), arrayEntry(setArrayEntry), valid(false), type(Type::UNKNOWN_TYPE), typeSize(
				0), arraySize(0), sizeTillEnd(0), readWriteMode(setReadWriteMode) {
	memset(value, 0, sizeof(value));
	if (data_set != NULL) {
		data_set->registerVariable(this);
	}
}

PoolRawAccess::~PoolRawAccess() {}

ReturnValue_t PoolRawAccess::read() {
	ReturnValue_t result = RETURN_FAILED;
	PoolEntryIF* read_out = ::dataPool.getRawData(dataPoolId);
	if (read_out != NULL) {
		result = handleReadOut(read_out);
		if(result == RETURN_OK) {
			return result;
		}
	} else {
		result = READ_ENTRY_NON_EXISTENT;
	}
	handleReadError(result);
	return result;
}

ReturnValue_t PoolRawAccess::handleReadOut(PoolEntryIF* read_out) {
	ReturnValue_t result = RETURN_FAILED;
	valid = read_out->getValid();
	if (read_out->getSize() > arrayEntry) {
		arraySize = read_out->getSize();
		typeSize = read_out->getByteSize() / read_out->getSize();
		type = read_out->getType();
		if (typeSize <= sizeof(value)) {
			uint16_t arrayPosition = arrayEntry * typeSize;
			sizeTillEnd = read_out->getByteSize() - arrayPosition;
			uint8_t* ptr = &((uint8_t*) read_out->getRawData())[arrayPosition];
			memcpy(value, ptr, typeSize);
			return RETURN_OK;
		} else {
			result = READ_TYPE_TOO_LARGE;
		}
	} else {
		//debug << "PoolRawAccess: Size: " << (int)read_out->getSize() << std::endl;
		result = READ_INDEX_TOO_LARGE;
	}
	return result;
}

void PoolRawAccess::handleReadError(ReturnValue_t result) {
	sif::error << "PoolRawAccess: read of DP Variable 0x" << std::hex << dataPoolId
			<< std::dec << " failed, ";
	if(result == READ_TYPE_TOO_LARGE) {
		sif::error << "type too large." << std::endl;
	}
	else if(result == READ_INDEX_TOO_LARGE) {
		sif::error << "index too large." << std::endl;
	}
	else if(result == READ_ENTRY_NON_EXISTENT) {
		sif::error << "entry does not exist." << std::endl;
	}

	valid = INVALID;
	typeSize = 0;
	sizeTillEnd = 0;
	memset(value, 0, sizeof(value));
}

ReturnValue_t PoolRawAccess::commit() {
	PoolEntryIF* write_back = ::dataPool.getRawData(dataPoolId);
	if ((write_back != NULL) && (readWriteMode != VAR_READ)) {
		write_back->setValid(valid);
		uint8_t array_position = arrayEntry * typeSize;
		uint8_t* ptr = &((uint8_t*) write_back->getRawData())[array_position];
		memcpy(ptr, value, typeSize);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

uint8_t* PoolRawAccess::getEntry() {
	return value;
}

ReturnValue_t PoolRawAccess::getEntryEndianSafe(uint8_t* buffer,
		uint32_t* writtenBytes, uint32_t max_size) {
	uint8_t* data_ptr = getEntry();
//	debug << "PoolRawAccess::getEntry: Array position: " << index * size_of_type << " Size of T: " << (int)size_of_type << " ByteSize: " << byte_size << " Position: " << *size << std::endl;
	if (typeSize == 0)
		return DATA_POOL_ACCESS_FAILED;
	if (typeSize > max_size)
		return INCORRECT_SIZE;
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
	for (uint8_t count = 0; count < typeSize; count++) {
		buffer[count] = data_ptr[typeSize - count - 1];
	}
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
	memcpy(buffer, data_ptr, typeSize);
#endif
	*writtenBytes = typeSize;
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t PoolRawAccess::serialize(uint8_t** buffer, size_t* size,
		const size_t max_size, bool bigEndian) const {
	if (typeSize + *size <= max_size) {
		if (bigEndian) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
			for (uint8_t count = 0; count < typeSize; count++) {
				(*buffer)[count] = value[typeSize - count - 1];
			}
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
			memcpy(*buffer, value, typeSize);
#endif
		} else {
			memcpy(*buffer, value, typeSize);
		}
		*size += typeSize;
		(*buffer) += typeSize;
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SerializeIF::BUFFER_TOO_SHORT;
	}
}


Type PoolRawAccess::getType() {
	return type;
}

uint8_t PoolRawAccess::getSizeOfType() {
	return typeSize;
}

uint8_t PoolRawAccess::getArraySize(){
	return arraySize;
}

uint32_t PoolRawAccess::getDataPoolId() const {
	return dataPoolId;
}

PoolVariableIF::ReadWriteMode_t PoolRawAccess::getReadWriteMode() const {
	return readWriteMode;
}

ReturnValue_t PoolRawAccess::setEntryFromBigEndian(const uint8_t* buffer,
		uint32_t setSize) {
	if (typeSize == setSize) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
		for (uint8_t count = 0; count < typeSize; count++) {
			value[count] = buffer[typeSize - count - 1];
		}
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
		memcpy(value, buffer, typeSize);
#endif
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		sif::error << "PoolRawAccess::setEntryFromBigEndian: Illegal sizes: Internal"
				<< (uint32_t) typeSize << ", Requested: " << setSize
				<< std::endl;
		return INCORRECT_SIZE;
	}
}

bool PoolRawAccess::isValid() const {
	if (valid != INVALID)
		return true;
	else
		return false;
}

void PoolRawAccess::setValid(uint8_t valid) {
	this->valid = valid;
}

uint16_t PoolRawAccess::getSizeTillEnd() const {
	return sizeTillEnd;
}


size_t PoolRawAccess::getSerializedSize() const {
	return typeSize;
}

ReturnValue_t PoolRawAccess::deSerialize(const uint8_t** buffer, size_t* size,
		bool bigEndian) {
	// TODO: Needs to be tested!!!
	if (*size >= typeSize) {
		*size -= typeSize;
		if (bigEndian) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
			for (uint8_t count = 0; count < typeSize; count++) {
				value[count] = (*buffer)[typeSize - count - 1];
			}
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
			memcpy(value, *buffer, typeSize);
#endif
		}
		else {
			memcpy(value, *buffer, typeSize);
		}
		*buffer += typeSize;
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return SerializeIF::STREAM_TOO_SHORT;
	}
}
