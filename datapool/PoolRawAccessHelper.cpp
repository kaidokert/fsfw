/**
 * @file PoolRawAccessHelper.cpp
 *
 * @date 22.12.2019
 * @author R. Mueller
 */

#include <framework/datapool/PoolRawAccessHelper.h>
#include <framework/datapool/DataSet.h>

PoolRawAccessHelper::PoolRawAccessHelper(uint8_t * poolIdBuffer_,
		uint8_t  numberOfParameters_):
		poolIdBuffer(poolIdBuffer_),
		numberOfParameters(numberOfParameters_), validBufferIndex(0), validBufferIndexBit(1){
}

PoolRawAccessHelper::~PoolRawAccessHelper() {
}

ReturnValue_t PoolRawAccessHelper::serialize(uint8_t **buffer, uint32_t *size,
		const uint32_t max_size, bool bigEndian) {
	ReturnValue_t result;
	int32_t remainingParametersSize = numberOfParameters * 4;
	for(uint8_t count=0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(buffer,
				&remainingParametersSize,size,max_size, bigEndian, false);
		if(result != RETURN_OK) {
			return result;
		}
	}
	if(remainingParametersSize != 0) {
		debug << "Pool Raw Access: Remaining parameters size not 0 !" << std::endl;
		result = RETURN_FAILED;
	}
	return result;
}

ReturnValue_t PoolRawAccessHelper::serializeWithValidityMask(uint8_t **buffer,
		uint32_t *size, const uint32_t max_size, bool bigEndian) {
	ReturnValue_t result;
	int32_t remainingParametersSize = numberOfParameters * 4;
	uint8_t validityMaskSize = numberOfParameters/8;
	uint8_t validityMask[validityMaskSize];
	memset(validityMask,0, validityMaskSize);
	for(uint8_t count = 0; count < numberOfParameters; count++) {
		result = serializeCurrentPoolEntryIntoBuffer(buffer,
				&remainingParametersSize,size,max_size, bigEndian,true,validityMask);
		if (result != RETURN_OK) {
			return result;
		}
	}
	if(remainingParametersSize != 0) {
		debug << "Pool Raw Access: Remaining parameters size not 0 !" << std::endl;
		result = RETURN_FAILED;
	}
	memcpy(*buffer + *size, validityMask, validityMaskSize);
	*size += validityMaskSize;
	validBufferIndex = 1;
	validBufferIndexBit = 0;
	return result;
}

ReturnValue_t PoolRawAccessHelper::serializeCurrentPoolEntryIntoBuffer(uint8_t ** buffer,
		int32_t * remainingParameters, uint32_t * hkDataSize,
		const uint32_t max_size, bool bigEndian, bool withValidMask, uint8_t * validityMask) {
	uint32_t currentPoolId;
	DataSet currentDataSet = DataSet();
	// Deserialize current pool ID from pool ID buffer
	ReturnValue_t result = AutoSerializeAdapter::deSerialize(&currentPoolId,
			&poolIdBuffer,remainingParameters,true);
	if(result != RETURN_OK) {
		debug << std::hex << "Pool Raw Access Helper: Error deSeralizing pool IDs" << std::dec << std::endl;
		return result;
	}

	// info << "Pool Raw Access Helper: Handling Pool ID: " << std::hex <<  currentPoolId << std::endl;
	PoolRawAccess currentPoolRawAccess(currentPoolId,0,&currentDataSet,PoolVariableIF::VAR_READ);

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

	result = currentDataSet.read();
	if (result != RETURN_OK) {
		debug << std::hex << "Pool Raw Access Helper: Error read raw dataset" << std::dec << std::endl;
		return result;
	}
	result = currentDataSet.serialize(buffer, hkDataSize,
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
