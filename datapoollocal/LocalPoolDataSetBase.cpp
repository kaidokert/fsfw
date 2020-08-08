#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalPoolDataSetBase.h>
#include <framework/serialize/SerializeAdapter.h>

#include <cmath>
#include <cstring>

LocalPoolDataSetBase::LocalPoolDataSetBase(HasLocalDataPoolIF *hkOwner,
        PoolVariableIF** registeredVariablesArray,
        const size_t maxNumberOfVariables):
        PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables) {
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalPoolDataSetBase::LocalPoolDataSetBase(object_id_t ownerId,
        PoolVariableIF** registeredVariablesArray,
        const size_t maxNumberOfVariables):
        PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables)  {
    HasLocalDataPoolIF* hkOwner = objectManager->get<HasLocalDataPoolIF>(
            ownerId);
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalPoolDataSetBase::~LocalPoolDataSetBase() {
}

ReturnValue_t LocalPoolDataSetBase::lockDataPool(uint32_t timeoutMs) {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->lockMutex(MutexIF::TimeoutType::WAITING, timeoutMs);
}

ReturnValue_t LocalPoolDataSetBase::serializeWithValidityBuffer(uint8_t **buffer,
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

ReturnValue_t LocalPoolDataSetBase::deSerializeWithValidityBuffer(
        const uint8_t **buffer, size_t *size,
        SerializeIF::Endianness streamEndianness) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    for (uint16_t count = 0; count < fillCount; count++) {
        result = registeredVariables[count]->deSerialize(buffer, size,
                        streamEndianness);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }
    uint8_t validBufferIndex = 0;
    uint8_t validBufferIndexBit = 0;
    // could be made more efficient but make it work first
    for (uint16_t count = 0; count < fillCount; count++) {
        // set validity buffer here.
        bool nextVarValid = this->bitGetter(*buffer +
                validBufferIndex, validBufferIndexBit);
        registeredVariables[count]->setValid(nextVarValid);

        if(validBufferIndexBit == 7) {
            validBufferIndex ++;
            validBufferIndexBit = 0;
        }
        else {
            validBufferIndexBit ++;
        }
    }
    return result;
}
ReturnValue_t LocalPoolDataSetBase::unlockDataPool() {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->unlockMutex();
}

ReturnValue_t LocalPoolDataSetBase::serializeLocalPoolIds(uint8_t** buffer,
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

void LocalPoolDataSetBase::bitSetter(uint8_t* byte, uint8_t position) const {
    if(position > 7) {
        sif::debug << "Pool Raw Access: Bit setting invalid position" << std::endl;
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte |= 1 << shiftNumber;
}

size_t LocalPoolDataSetBase::getSerializedSize() const {
    if(withValidityBuffer) {
        uint8_t validityMaskSize = std::ceil(static_cast<float>(fillCount)/8.0);
        return validityMaskSize + PoolDataSetBase::getSerializedSize();
    }
    else {
        return PoolDataSetBase::getSerializedSize();
    }
}

void LocalPoolDataSetBase::setValidityBufferGeneration(
        bool withValidityBuffer) {
    this->withValidityBuffer = withValidityBuffer;
}

ReturnValue_t LocalPoolDataSetBase::deSerialize(const uint8_t **buffer,
        size_t *size, SerializeIF::Endianness streamEndianness) {
    if(withValidityBuffer) {
        return this->deSerializeWithValidityBuffer(buffer, size,
                streamEndianness);
    }
    else {
        return PoolDataSetBase::deSerialize(buffer, size, streamEndianness);
    }
}

ReturnValue_t LocalPoolDataSetBase::serialize(uint8_t **buffer, size_t *size,
        size_t maxSize, SerializeIF::Endianness streamEndianness) const {
    if(withValidityBuffer) {
        return this->serializeWithValidityBuffer(buffer, size,
                maxSize, streamEndianness);
    }
    else {
        return PoolDataSetBase::serialize(buffer, size, maxSize,
                streamEndianness);
    }
}

bool LocalPoolDataSetBase::bitGetter(const uint8_t* byte, uint8_t position) const {
    if(position > 7) {
        sif::debug << "Pool Raw Access: Bit setting invalid position" << std::endl;
        return false;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    return *byte & (1 << shiftNumber);
}

bool LocalPoolDataSetBase::isValid() const {
    return this->valid;
}


