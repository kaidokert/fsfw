/*
 * MonitoringListAdapter.cpp
 *
 *  Created on: 09.07.2014
 *      Author: baetz
 */

#include <framework/monitoring/MonitoringListAdapter.h>

ReturnValue_t MonitoringListAdapter::check() {
	LinkedElement<MonitoringIF>* element = start;
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	bool atLeastOneFailed = false;
	while ((element != NULL)) {
		result = element->value->check();
		if (result != HasReturnvaluesIF::RETURN_OK) {
			atLeastOneFailed = true;
		}
		element = element->getNext();
	}
	return (atLeastOneFailed) ?
			(ReturnValue_t) HasReturnvaluesIF::RETURN_FAILED :
			(ReturnValue_t) HasReturnvaluesIF::RETURN_OK;
}

//ReturnValue_t MonitoringListAdapter::serialize(uint8_t** buffer, uint32_t* size,
//		const uint32_t max_size, bool bigEndian) const {
//	return SerialListAdapter<MonitoringIF>::serialize(start, buffer, size, max_size, bigEndian);
//}
//
//uint32_t MonitoringListAdapter::getSerializedSize() const {
//	return SerialListAdapter<MonitoringIF>::getSerializedSize(start);
//}
//
//ReturnValue_t MonitoringListAdapter::deSerialize(const uint8_t** buffer, int32_t* size,
//		bool bigEndian) {
//	return SerialListAdapter<MonitoringIF>::deSerialize(start, buffer, size, bigEndian);
//}

ReturnValue_t MonitoringListAdapter::updateMonitor(uint32_t parameterId,
		uint8_t limitType, const uint8_t* data, uint32_t size) {
	LinkedElement<MonitoringIF>* element = start;
	while ((element != NULL)) {
		if ((element->value->getLimitId() == parameterId)) {
			return element->value->setLimits(limitType, data, size);
		}
		element = element->getNext();
	}
	return MonitoringIF::MONITOR_NOT_FOUND;
}

ReturnValue_t MonitoringListAdapter::setChecking(uint8_t strategy, uint32_t parameterId) {
	LinkedElement<MonitoringIF>* element = start;
	while ((element != NULL)) {
		if ((element->value->getLimitId() == parameterId)) {
			return element->value->setChecking(strategy);
		}
		element = element->getNext();
	}
	return MonitoringIF::MONITOR_NOT_FOUND;
}

ReturnValue_t MonitoringListAdapter::setChecking(uint8_t strategy) {
	LinkedElement<MonitoringIF>* element = start;
	ReturnValue_t result = MonitoringIF::MONITOR_NOT_FOUND;
	bool atLeastOneUpdated = false;
	while ((element != NULL)) {
		result = element->value->setChecking(strategy);
		if (result == HasReturnvaluesIF::RETURN_OK) {
			atLeastOneUpdated = true;
		}
		element = element->getNext();
	}
	result =
			(atLeastOneUpdated) ?
					(ReturnValue_t) HasReturnvaluesIF::RETURN_OK :
					(ReturnValue_t) MonitoringIF::MONITOR_NOT_FOUND;
	return result;
}
