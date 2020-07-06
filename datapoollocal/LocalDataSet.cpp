#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/serialize/SerializeAdapter.h>

#include <cmath>
#include <cstring>

LocalDataSet::LocalDataSet(OwnsLocalDataPoolIF *hkOwner,
        const size_t maxNumberOfVariables):
        DataSetBase(poolVarList.data(), maxNumberOfVariables) {
    poolVarList.reserve(maxNumberOfVariables);
    poolVarList.resize(maxNumberOfVariables);
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalDataSet::LocalDataSet(object_id_t ownerId,
        const size_t maxNumberOfVariables):
        DataSetBase(poolVarList.data(), maxNumberOfVariables)  {
    poolVarList.reserve(maxNumberOfVariables);
    poolVarList.resize(maxNumberOfVariables);
    OwnsLocalDataPoolIF* hkOwner = objectManager->get<OwnsLocalDataPoolIF>(
            ownerId);
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalDataSet::~LocalDataSet() {
}

ReturnValue_t LocalDataSet::lockDataPool(uint32_t timeoutMs) {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->lockMutex(timeoutMs);
}

ReturnValue_t LocalDataSet::serializeWithValidityBuffer(uint8_t **buffer,
        size_t *size, size_t maxSize,
        SerializeIF::Endianness streamEndianness) const {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    uint8_t validityMaskSize = std::ceil(static_cast<float>(fillCount)/8.0);
    uint8_t validityMask[validityMaskSize];
    uint8_t validBufferIndex = 0;
    uint8_t validBufferIndexBit = 0;
    for (uint16_t count = 0; count < fillCount; count++) {
        if(registeredVariables[count]->isValid()) {
            // set validity buffer here.
            this->bitSetter(validityMask + validBufferIndex,
                    validBufferIndexBit);
            if(validBufferIndexBit == 7) {
                validBufferIndex ++;
                validBufferIndexBit = 0;
            }
            else {
                validBufferIndexBit ++;
            }
        }
        result = registeredVariables[count]->serialize(buffer, size, maxSize,
                streamEndianness);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }
    // copy validity buffer to end
    std::memcpy(*buffer, validityMask, validityMaskSize);
    *size += validityMaskSize;
    return result;
}

ReturnValue_t LocalDataSet::unlockDataPool() {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->unlockMutex();
}

ReturnValue_t LocalDataSet::serializeLocalPoolIds(uint8_t** buffer,
        size_t* size, size_t maxSize,
        SerializeIF::Endianness streamEndianness) const {
    for (uint16_t count = 0; count < fillCount; count++) {
        lp_id_t currentPoolId = registeredVariables[count]->getDataPoolId();
        auto result = SerializeAdapter::serialize(&currentPoolId, buffer,
                size, maxSize, streamEndianness);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            sif::warning << "LocalDataSet::serializeLocalPoolIds: Serialization"
                    " error!" << std::endl;
            return result;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void LocalDataSet::bitSetter(uint8_t* byte, uint8_t position) const {
    if(position > 7) {
        sif::debug << "Pool Raw Access: Bit setting invalid position" << std::endl;
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte |= 1 << shiftNumber;
}

