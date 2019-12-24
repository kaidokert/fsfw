/**
 * @file PoolRawAccessHelper.cpp
 *
 * @date 22.12.2019
 * @author R. Mueller
 */

#include <framework/datapool/PoolRawAccessHelper.h>

PoolRawAccessHelper::PoolRawAccessHelper(DataSet *dataSet_,
		const uint8_t * poolIdBuffer_, uint8_t  numberOfParameters_):
		dataSet(dataSet_), poolIdBuffer(poolIdBuffer_),
		numberOfParameters(numberOfParameters_), validBufferIndex(0), validBufferIndexBit(1){
}

PoolRawAccessHelper::~PoolRawAccessHelper() {
}

ReturnValue_t PoolRawAccessHelper::serialize(uint8_t **buffer, uint32_t *size,
		const uint32_t max_size, bool bigEndian) {
	ReturnValue_t result;
	const uint8_t ** pPoolIdBuffer = &poolIdBuffer;
	int32_t remainingParametersSize = numberOfParameters * 4;
	for(uint8_t count=0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(pPoolIdBuffer,buffer,
				&remainingParametersSize,size,max_size, bigEndian, false);
		if(result != RETURN_OK) {
			return result;
		}
	}
	return RETURN_OK;
}

ReturnValue_t PoolRawAccessHelper::serializeWithValidityMask(uint8_t **buffer,
		uint32_t *size, const uint32_t max_size, bool bigEndian) {
	ReturnValue_t result;
	const uint8_t ** pPoolIdBuffer = &poolIdBuffer;
	int32_t remainingParametersSize = numberOfParameters * 4;
	uint8_t validityMaskSize = numberOfParameters/8;
	uint8_t validityMask[validityMaskSize];
	memset(validityMask,0, validityMaskSize);
	for(uint8_t count=0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(pPoolIdBuffer,buffer,
				&remainingParametersSize,size,max_size, bigEndian,true,validityMask);
		if (result != RETURN_OK) {
			return result;
		}
	}
	memcpy(*buffer + *size, validityMask, validityMaskSize);
	*size += validityMaskSize;
	validBufferIndex = 1;
	validBufferIndexBit = 0;
	return RETURN_OK;
}

ReturnValue_t PoolRawAccessHelper::serializeCurrentPoolEntryIntoBuffer(const uint8_t ** pPoolIdBuffer,
		uint8_t ** buffer, int32_t * remainingParameters, uint32_t * hkDataSize,
		const uint32_t max_size, bool bigEndian, bool withValidMask, uint8_t * validityMask) {
	uint32_t currentPoolId;
	// Deserialize current pool ID from pool ID buffer
	ReturnValue_t result = AutoSerializeAdapter::deSerialize(&currentPoolId,
			pPoolIdBuffer,remainingParameters,true);
	if(result != RETURN_OK) {
		debug << std::hex << "Pool Raw Access Helper: Error deSeralizing pool IDs" << std::dec << std::endl;
		return result;
	}
	PoolRawAccess currentPoolRawAccess(currentPoolId,0,dataSet,PoolVariableIF::VAR_READ);

	// set valid mask bit if necessary
	if(withValidMask) {
		if(currentPoolRawAccess.isValid()) {
			validityMask[validBufferIndex] =
					bitSetter(validityMask[validBufferIndex], validBufferIndexBit, true);
			validBufferIndexBit ++;
			if(validBufferIndexBit == 8) {
				validBufferIndex ++;
				validBufferIndexBit = 1;
			}
		}
	}

	result = dataSet->read();
	if (result != RETURN_OK) {
		debug << std::hex << "Pool Raw Access Helper: Error read raw dataset" << std::dec << std::endl;
		return result;
	}
	result = dataSet->serialize(buffer, hkDataSize,
			max_size, bigEndian);
	if (result != RETURN_OK) {
		debug << "Pool Raw Access Helper: Error serializing pool data into send buffer" << std::endl;
	}
	return result;
}

uint8_t PoolRawAccessHelper::bitSetter(uint8_t byte, uint8_t position, bool value) {
	if(position < 1 or position > 8) {
		debug << "Pool Raw Access: Bit setting invalid position" << std::endl;
		return byte;
	}
	uint8_t shiftNumber = position + (6 - 2 * (position - 1));
	byte = (byte | value) <<  shiftNumber;
	return byte;
}
