#include "LocalPoolDataSetBase.h"
#include "HasLocalDataPoolIF.h"
#include "internal/HasLocalDpIFUserAttorney.h"

#include "../serviceinterface/ServiceInterface.h"
#include "../datapoollocal/LocalDataPoolManager.h"
#include "../housekeeping/PeriodicHousekeepingHelper.h"
#include "../serialize/SerializeAdapter.h"

#include <cmath>
#include <cstring>

LocalPoolDataSetBase::LocalPoolDataSetBase(HasLocalDataPoolIF *hkOwner,
		uint32_t setId, PoolVariableIF** registeredVariablesArray,
		const size_t maxNumberOfVariables, bool periodicHandling):
		PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables) {
	if(hkOwner == nullptr) {
		// Configuration error.
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "LocalPoolDataSetBase::LocalPoolDataSetBase: Owner "
				<< "invalid!" << std::endl;
#else
		sif::printError("LocalPoolDataSetBase::LocalPoolDataSetBase: Owner "
				"invalid!\n\r");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
		return;
	}
	AccessPoolManagerIF* accessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);

	if(accessor != nullptr) {
		poolManager = accessor->getHkManagerHandle();
		mutexIfSingleDataCreator = accessor->getLocalPoolMutex();
	}

	this->sid.objectId = hkOwner->getObjectId();
	this->sid.ownerSetId = setId;

	// Data creators get a periodic helper for periodic HK data generation.
	if(periodicHandling) {
		periodicHelper = new PeriodicHousekeepingHelper(this);
	}
}

LocalPoolDataSetBase::LocalPoolDataSetBase(sid_t sid,
		PoolVariableIF** registeredVariablesArray,
		const size_t maxNumberOfVariables):
        PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables)  {
	HasLocalDataPoolIF* hkOwner = objectManager->get<HasLocalDataPoolIF>(
			sid.objectId);
	if(hkOwner != nullptr) {
		AccessPoolManagerIF* accessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);
		if(accessor != nullptr) {
			mutexIfSingleDataCreator = accessor->getLocalPoolMutex();
		}
	}

	this->sid = sid;
}

LocalPoolDataSetBase::LocalPoolDataSetBase(
		PoolVariableIF **registeredVariablesArray,
		const size_t maxNumberOfVariables, bool protectEveryReadCommitCall):
		PoolDataSetBase(registeredVariablesArray, maxNumberOfVariables) {
	this->setReadCommitProtectionBehaviour(protectEveryReadCommitCall);
}


LocalPoolDataSetBase::~LocalPoolDataSetBase() {
	if(periodicHelper != nullptr) {
		delete periodicHelper;
	}
}

ReturnValue_t LocalPoolDataSetBase::lockDataPool(
		MutexIF::TimeoutType timeoutType,
		uint32_t timeoutMs) {
	if(mutexIfSingleDataCreator != nullptr) {
		return mutexIfSingleDataCreator->lockMutex(timeoutType, timeoutMs);
	}
	return HasReturnvaluesIF::RETURN_OK;
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
	if(mutexIfSingleDataCreator != nullptr) {
		return mutexIfSingleDataCreator->unlockMutex();
	}
	return HasReturnvaluesIF::RETURN_OK;
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "LocalPoolDataSetBase::serializeLocalPoolIds: "
				<< "Serialization error!" << std::endl;
#else
		sif::printWarning("LocalPoolDataSetBase::serializeLocalPoolIds: "
				"Serialization error!\n\r");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "LocalPoolDataSetBase::bitSetter: Invalid position!"
		<< std::endl;
#else
		sif::printWarning("LocalPoolDataSetBase::bitSetter: "
				"Invalid position!\n\r");
#endif
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
	this->changed = changed;
}

bool LocalPoolDataSetBase::hasChanged() const {
	return changed;
}

sid_t LocalPoolDataSetBase::getSid() const {
	return sid;
}

bool LocalPoolDataSetBase::bitGetter(const uint8_t* byte,
		uint8_t position) const {
	if(position > 7) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::debug << "Pool Raw Access: Bit setting invalid position"
				<< std::endl;
#endif
		return false;
	}
	uint8_t shiftNumber = position + (7 - 2 * position);
	return *byte & (1 << shiftNumber);
}

bool LocalPoolDataSetBase::isValid() const {
	return this->valid;
}

void LocalPoolDataSetBase::setValidity(bool valid, bool setEntriesRecursively) {
	if(setEntriesRecursively) {
		for(size_t idx = 0; idx < this->getFillCount(); idx++) {
			registeredVariables[idx] -> setValid(valid);
		}
	}
	this->valid = valid;
}

object_id_t LocalPoolDataSetBase::getCreatorObjectId() {
	if(poolManager != nullptr) {
		return poolManager->getCreatorObjectId();
	}
	return objects::NO_OBJECT;
}
