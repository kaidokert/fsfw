/**
 * @file	PowerComponent.cpp
 * @brief	This file defines the PowerComponent class.
 * @date	28.08.2014
 * @author	baetz
 */

#include "PowerComponent.h"

PowerComponent::PowerComponent() :
		deviceObjectId(0), switchId1(0xFF), switchId2(0xFF), doIHaveTwoSwitches(
		false), min(0.0), max(0.0), moduleId(0) {
}
PowerComponent::PowerComponent(object_id_t setId, uint8_t moduleId, float min, float max,
		uint8_t switchId1, bool twoSwitches, uint8_t switchId2) :
		deviceObjectId(setId), switchId1(switchId1), switchId2(switchId2), doIHaveTwoSwitches(
				twoSwitches), min(min), max(max), moduleId(moduleId) {
}

ReturnValue_t PowerComponent::serialize(uint8_t** buffer, size_t* size,
		size_t maxSize, Endianness streamEndianness) const {
	ReturnValue_t result = SerializeAdapter::serialize(&min, buffer,
			size, maxSize, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return SerializeAdapter::serialize(&max, buffer, size, maxSize,
			streamEndianness);
}

size_t PowerComponent::getSerializedSize() const {
	return sizeof(min) + sizeof(max);
}

object_id_t PowerComponent::getDeviceObjectId() {
	return deviceObjectId;
}

uint8_t PowerComponent::getSwitchId1() {
	return switchId1;
}

uint8_t PowerComponent::getSwitchId2() {
	return switchId2;
}

bool PowerComponent::hasTwoSwitches() {
	return doIHaveTwoSwitches;
}

float PowerComponent::getMin() {
	return min;
}

float PowerComponent::getMax() {
	return max;
}

ReturnValue_t PowerComponent::deSerialize(const uint8_t** buffer, size_t* size,
Endianness streamEndianness) {
	ReturnValue_t result = SerializeAdapter::deSerialize(&min, buffer,
			size, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return SerializeAdapter::deSerialize(&max, buffer, size, streamEndianness);
}

ReturnValue_t PowerComponent::getParameter(uint8_t domainId,
		uint16_t parameterId, ParameterWrapper* parameterWrapper,
		const ParameterWrapper* newValues, uint16_t startAtIndex) {
	if (domainId != moduleId) {
		return INVALID_DOMAIN_ID;
	}
	switch (parameterId) {
	case 0:
		parameterWrapper->set<>(min);
		break;
	case 1:
		parameterWrapper->set<>(max);
		break;
	default:
		return INVALID_MATRIX_ID;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
