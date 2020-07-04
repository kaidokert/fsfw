#include <framework/datapoolglob/GlobalDataPool.h>
#include <framework/datapoolglob/PoolRawAccess.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/serialize/EndianConverter.h>

#include <cstring>

PoolRawAccess::PoolRawAccess(uint32_t set_id, uint8_t setArrayEntry,
		DataSetIF* dataSet, ReadWriteMode_t setReadWriteMode) :
		dataPoolId(set_id), arrayEntry(setArrayEntry), valid(false),
		type(Type::UNKNOWN_TYPE), typeSize(0), arraySize(0), sizeTillEnd(0),
		readWriteMode(setReadWriteMode) {
	memset(value, 0, sizeof(value));
	if (dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

PoolRawAccess::~PoolRawAccess() {}

ReturnValue_t PoolRawAccess::read(uint32_t lockTimeout) {
	ReturnValue_t result = glob::dataPool.lockDataPool(lockTimeout);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = readWithoutLock();
	ReturnValue_t unlockResult = glob::dataPool.unlockDataPool();
	if(unlockResult != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "GlobPoolVar::read: Could not unlock global data pool"
				<< std::endl;
	}
	return result;
}

ReturnValue_t PoolRawAccess::readWithoutLock() {
	ReturnValue_t result = RETURN_FAILED;
	PoolEntryIF* readOut = glob::dataPool.getRawData(dataPoolId);
	if (readOut != nullptr) {
		result = handleReadOut(readOut);
		if(result == RETURN_OK) {
			return result;
		}
	} else {
		result = READ_ENTRY_NON_EXISTENT;
	}
	handleReadError(result);
	return result;
}

ReturnValue_t PoolRawAccess::handleReadOut(PoolEntryIF* readOut) {
	ReturnValue_t result = RETURN_FAILED;
	valid = readOut->getValid();
	if (readOut->getSize() > arrayEntry) {
		arraySize = readOut->getSize();
		typeSize = readOut->getByteSize() / readOut->getSize();
		type = readOut->getType();
		if (typeSize <= sizeof(value)) {
			uint16_t arrayPosition = arrayEntry * typeSize;
			sizeTillEnd = readOut->getByteSize() - arrayPosition;
			uint8_t* ptr = &((uint8_t*) readOut->getRawData())[arrayPosition];
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

ReturnValue_t PoolRawAccess::commit(uint32_t lockTimeout) {
	ReturnValue_t result = glob::dataPool.lockDataPool(lockTimeout);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = commitWithoutLock();
	ReturnValue_t unlockResult = glob::dataPool.unlockDataPool();
	if(unlockResult != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "GlobPoolVar::read: Could not unlock global data pool"
				<< std::endl;
	}
	return result;
}

ReturnValue_t PoolRawAccess::commitWithoutLock() {
	PoolEntryIF* write_back = glob::dataPool.getRawData(dataPoolId);
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
	// debug << "PoolRawAccess::getEntry: Array position: " <<
	// 	      index * size_of_type << " Size of T: " << (int)size_of_type <<
	//		  " ByteSize: " << byte_size << " Position: " << *size << std::endl;
	if (typeSize == 0)
		return DATA_POOL_ACCESS_FAILED;
	if (typeSize > max_size)
		return INCORRECT_SIZE;
	EndianConverter::convertBigEndian(buffer, data_ptr, typeSize);
	*writtenBytes = typeSize;
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t PoolRawAccess::serialize(uint8_t** buffer, size_t* size,
        size_t maxSize, Endianness streamEndianness) const {
    if (typeSize + *size <= maxSize) {
    	switch(streamEndianness) {
    	case(Endianness::BIG):
    		EndianConverter::convertBigEndian(*buffer, value, typeSize);
    		break;
    	case(Endianness::LITTLE):
			EndianConverter::convertLittleEndian(*buffer, value, typeSize);
    		break;
    	case(Endianness::MACHINE):
    	default:
            memcpy(*buffer, value, typeSize);
            break;
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

size_t PoolRawAccess::getSizeOfType() {
	return typeSize;
}

size_t PoolRawAccess::getArraySize(){
	return arraySize;
}

uint32_t PoolRawAccess::getDataPoolId() const {
	return dataPoolId;
}

PoolVariableIF::ReadWriteMode_t PoolRawAccess::getReadWriteMode() const {
	return readWriteMode;
}

ReturnValue_t PoolRawAccess::setEntryFromBigEndian(const uint8_t *buffer,
		size_t setSize) {
	if (typeSize == setSize) {
		EndianConverter::convertBigEndian(value, buffer, typeSize);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		sif::error << "PoolRawAccess::setEntryFromBigEndian: Illegal sizes: "
				"Internal" << (uint32_t) typeSize << ", Requested: " << setSize
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

void PoolRawAccess::setValid(bool valid) {
	this->valid = valid;
}

uint16_t PoolRawAccess::getSizeTillEnd() const {
	return sizeTillEnd;
}


size_t PoolRawAccess::getSerializedSize() const {
	return typeSize;
}

ReturnValue_t PoolRawAccess::deSerialize(const uint8_t **buffer, size_t *size,
        Endianness streamEndianness) {

    if (*size >= typeSize) {
        switch(streamEndianness) {
        case(Endianness::BIG):
			EndianConverter::convertBigEndian(value, *buffer, typeSize);
		    break;
        case(Endianness::LITTLE):
			EndianConverter::convertLittleEndian(value, *buffer, typeSize);
		   	break;
        case(Endianness::MACHINE):
        default:
            memcpy(value, *buffer, typeSize);
            break;
        }
    	*size -= typeSize;
        *buffer += typeSize;
        return HasReturnvaluesIF::RETURN_OK;
    }
}
