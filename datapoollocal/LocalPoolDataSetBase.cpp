#include "LocalPoolDataSetBase.h"
#include "../datapoollocal/LocalDataPoolManager.h"
#include "../housekeeping/PeriodicHousekeepingHelper.h"
#include "../serialize/SerializeAdapter.h"

#include <cmath>
#include <cstring>

LocalPoolDataSetBase::LocalPoolDataSetBase(HasLocalDataPoolIF *hkOwner,
        uint32_t setId, PoolVariableIF** registeredVariablesArray,
        const size_t maxNumberOfVariables, bool noPeriodicHandling):
        PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables) {
    if(hkOwner == nullptr) {
        // Configuration error.
        sif::error << "LocalPoolDataSetBase::LocalPoolDataSetBase: Owner "
                << "invalid!" << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
    this->sid.objectId = hkOwner->getObjectId();
    this->sid.ownerSetId = setId;

    mutex = MutexFactory::instance()->createMutex();

    // Data creators get a periodic helper for periodic HK data generation.
    if(not noPeriodicHandling) {
        periodicHelper = new PeriodicHousekeepingHelper(this);
    }
}

LocalPoolDataSetBase::LocalPoolDataSetBase(sid_t sid,
        PoolVariableIF** registeredVariablesArray,
        const size_t maxNumberOfVariables):
        PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables)  {
    HasLocalDataPoolIF* hkOwner = objectManager->get<HasLocalDataPoolIF>(
            sid.objectId);
    if(hkOwner == nullptr) {
        // Configuration error.
        sif::error << "LocalPoolDataSetBase::LocalPoolDataSetBase: Owner "
                << "invalid!" << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
    this->sid = sid;

    mutex = MutexFactory::instance()->createMutex();
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

    if(*size + validityMaskSize > maxSize) {
        return SerializeIF::BUFFER_TOO_SHORT;
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

    if(*size < std::ceil(static_cast<float>(fillCount) / 8.0)) {
        return SerializeIF::STREAM_TOO_SHORT;
    }

    uint8_t validBufferIndex = 0;
    uint8_t validBufferIndexBit = 0;
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
        size_t* size, size_t maxSize,SerializeIF::Endianness streamEndianness,
        bool serializeFillCount) const {
    // Serialize as uint8_t
    uint8_t fillCount = this->fillCount;
    if(serializeFillCount) {
        SerializeAdapter::serialize(&fillCount, buffer, size, maxSize,
                streamEndianness);
    }
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


uint8_t LocalPoolDataSetBase::getLocalPoolIdsSerializedSize(
        bool serializeFillCount) const {
    if(serializeFillCount) {
        return fillCount * sizeof(lp_id_t) + sizeof(uint8_t);
    }
    else {
        return fillCount * sizeof(lp_id_t);
    }
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

void LocalPoolDataSetBase::bitSetter(uint8_t* byte, uint8_t position) const {
    if(position > 7) {
        sif::debug << "Pool Raw Access: Bit setting invalid position"
                << std::endl;
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte |= 1 << shiftNumber;
}

void LocalPoolDataSetBase::setDiagnostic(bool isDiagnostics) {
    this->diagnostic = isDiagnostics;
}

bool LocalPoolDataSetBase::isDiagnostics() const {
    return diagnostic;
}

void LocalPoolDataSetBase::setReportingEnabled(bool reportingEnabled) {
    this->reportingEnabled = reportingEnabled;
}

bool LocalPoolDataSetBase::getReportingEnabled() const {
    return reportingEnabled;
}

void LocalPoolDataSetBase::initializePeriodicHelper(
        float collectionInterval, dur_millis_t minimumPeriodicInterval,
        bool isDiagnostics, uint8_t nonDiagIntervalFactor) {
    periodicHelper->initialize(collectionInterval, minimumPeriodicInterval,
            isDiagnostics, nonDiagIntervalFactor);
}

void LocalPoolDataSetBase::setChanged(bool changed) {
    // TODO: Make this configurable?
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING, 20);
    this->changed = changed;
}

bool LocalPoolDataSetBase::hasChanged() const {
    // TODO: Make this configurable?
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING, 20);
    return changed;
}

sid_t LocalPoolDataSetBase::getSid() const {
    return sid;
}

bool LocalPoolDataSetBase::bitGetter(const uint8_t* byte,
        uint8_t position) const {
    if(position > 7) {
        sif::debug << "Pool Raw Access: Bit setting invalid position"
                << std::endl;
        return false;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    return *byte & (1 << shiftNumber);
}

bool LocalPoolDataSetBase::isValid() const {
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING, 5);
    return this->valid;
}

void LocalPoolDataSetBase::setValidity(bool valid, bool setEntriesRecursively) {
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING, 5);
    if(setEntriesRecursively) {
        for(size_t idx = 0; idx < this->getFillCount(); idx++) {
            registeredVariables[idx] -> setValid(valid);
        }
    }
    this->valid = valid;
}
