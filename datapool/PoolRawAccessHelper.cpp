/**
 * @file PoolRawAccessHelper.cpp
 *
 * @date 22.12.2019
 * @author R. Mueller
 */

#include "../datapool/PoolRawAccessHelper.h"
#include "../datapoolglob/GlobalDataSet.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

#include <cmath>
#include <cstring>

PoolRawAccessHelper::PoolRawAccessHelper(uint32_t * poolIdBuffer_,
		uint8_t  numberOfParameters_):
		poolIdBuffer(reinterpret_cast<uint8_t * >(poolIdBuffer_)),
		numberOfParameters(numberOfParameters_), validBufferIndex(0),
		validBufferIndexBit(1) {
}

PoolRawAccessHelper::~PoolRawAccessHelper() {
}

ReturnValue_t PoolRawAccessHelper::serialize(uint8_t **buffer, size_t *size,
		const size_t max_size, SerializeIF::Endianness streamEndianness) {
	SerializationArgs serializationArgs = {buffer, size, max_size,
	        streamEndianness};
	ReturnValue_t result = RETURN_OK;
	size_t remainingParametersSize = numberOfParameters * 4;
	for(uint8_t count=0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(serializationArgs,
				&remainingParametersSize, false);
		if(result != RETURN_OK) {
			return result;
		}
	}
	if(remainingParametersSize != 0) {
		sif::debug << "PoolRawAccessHelper: "
				"Remaining parameters size not 0 !" << std::endl;
		result = RETURN_FAILED;
	}
	return result;
}

ReturnValue_t PoolRawAccessHelper::serializeWithValidityMask(uint8_t ** buffer,
		size_t * size, const size_t max_size,
		SerializeIF::Endianness streamEndianness) {
	ReturnValue_t result = RETURN_OK;
	SerializationArgs argStruct = {buffer, size, max_size, streamEndianness};
	size_t remainingParametersSize = numberOfParameters * 4;
	uint8_t validityMaskSize = ceil((float)numberOfParameters/8.0);
	uint8_t validityMask[validityMaskSize];
	memset(validityMask,0, validityMaskSize);
	for(uint8_t count = 0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(argStruct,
				&remainingParametersSize,true,validityMask);
		if (result != RETURN_OK) {
			return result;
		}
	}
	if(remainingParametersSize != 0) {
		sif::debug << "PoolRawAccessHelper: Remaining "
				"parameters size not 0 !" << std::endl;
		result = RETURN_FAILED;
	}

	memcpy(*argStruct.buffer, validityMask, validityMaskSize);
	*size += validityMaskSize;
	validBufferIndex = 1;
	validBufferIndexBit = 0;
	return result;
}

ReturnValue_t PoolRawAccessHelper::serializeCurrentPoolEntryIntoBuffer(
		SerializationArgs argStruct, size_t * remainingParameters,
		bool withValidMask, uint8_t * validityMask) {
	uint32_t currentPoolId;
	// Deserialize current pool ID from pool ID buffer
	ReturnValue_t result = SerializeAdapter::deSerialize(&currentPoolId,
			&poolIdBuffer,remainingParameters, SerializeIF::Endianness::MACHINE);
	if(result != RETURN_OK) {
		sif::debug << std::hex << "PoolRawAccessHelper: Error deSeralizing "
				"pool IDs" << std::dec << std::endl;
		return result;
	}
	result = handlePoolEntrySerialization(currentPoolId, argStruct,
			withValidMask, validityMask);
	return result;
}

ReturnValue_t PoolRawAccessHelper::handlePoolEntrySerialization(
		uint32_t currentPoolId,SerializationArgs argStruct, bool withValidMask,
		uint8_t * validityMask) {
	ReturnValue_t result = RETURN_FAILED;
	uint8_t arrayPosition = 0;
	uint8_t counter = 0;
	bool poolEntrySerialized = false;
	//debug << "Pool Raw Access Helper: Handling Pool ID: "
	//      << std::hex <<  currentPoolId << std::endl;
	while(not poolEntrySerialized) {

		if(counter > GlobDataSet::DATA_SET_MAX_SIZE) {
			sif::error << "PoolRawAccessHelper: Config error, "
					 "max. number of possible data set variables exceeded"
				  << std::endl;
			return result;
		}
		counter ++;

		GlobDataSet currentDataSet;
		//debug << "Current array position: " << (int)arrayPosition << std::endl;
		PoolRawAccess currentPoolRawAccess(currentPoolId, arrayPosition,
				&currentDataSet, PoolVariableIF::VAR_READ);

		result = currentDataSet.read();
		if (result != RETURN_OK) {
			sif::debug << std::hex << "PoolRawAccessHelper: Error reading raw "
					"dataset with returncode 0x" << result << std::dec << std::endl;
			return result;
		}

		result = checkRemainingSize(&currentPoolRawAccess, &poolEntrySerialized,
				&arrayPosition);
		if(result != RETURN_OK) {
			sif::error << "Pool Raw Access Helper: Configuration Error at pool ID "
				  << std::hex << currentPoolId
				  << ". Size till end smaller than 0" << std::dec << std::endl;
			return result;
		}

		// set valid mask bit if necessary
		if(withValidMask) {
			if(currentPoolRawAccess.isValid()) {
				handleMaskModification(validityMask);
			}
			validBufferIndexBit ++;
		}

		result = currentDataSet.serialize(argStruct.buffer, argStruct.size,
				argStruct.max_size, argStruct.streamEndianness);
		if (result != RETURN_OK) {
			sif::debug << "Pool Raw Access Helper: Error serializing pool data with "
					 "ID 0x" << std::hex << currentPoolId << " into send buffer "
					 "with return code " << result << std::dec << std::endl;
			return result;
		}

	}
	return result;
}

ReturnValue_t PoolRawAccessHelper::checkRemainingSize(PoolRawAccess*
		currentPoolRawAccess, bool * isSerialized, uint8_t * arrayPosition) {
	int8_t remainingSize = currentPoolRawAccess->getSizeTillEnd() -
			currentPoolRawAccess->getSizeOfType();
	if(remainingSize == 0) {
		*isSerialized = true;
	}
	else if(remainingSize > 0) {
		*arrayPosition += 1;
	}
	else {
		return RETURN_FAILED;
	}
	return RETURN_OK;
}

void PoolRawAccessHelper::handleMaskModification(uint8_t * validityMask) {
	validityMask[validBufferIndex] =
			bitSetter(validityMask[validBufferIndex], validBufferIndexBit, true);
	if(validBufferIndexBit == 8) {
		validBufferIndex ++;
		validBufferIndexBit = 1;
	}
}

uint8_t PoolRawAccessHelper::bitSetter(uint8_t byte, uint8_t position,
		bool value) {
	if(position < 1 or position > 8) {
		sif::debug << "Pool Raw Access: Bit setting invalid position" << std::endl;
		return byte;
	}
	uint8_t shiftNumber = position + (6 - 2 * (position - 1));
	byte |= 1UL << shiftNumber;
	return byte;
}
