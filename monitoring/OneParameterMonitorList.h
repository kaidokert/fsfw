/*
 * OneParameterMonitorList.h
 *
 *  Created on: 25.07.2014
 *      Author: baetz
 */

#ifndef ONEPARAMETERMONITORLIST_H_
#define ONEPARAMETERMONITORLIST_H_

#include <framework/monitoring/MonitoringIF.h>
#include <framework/serialize/SerialLinkedListAdapter.h>

class OneParameterMonitorList: public SinglyLinkedList<MonitoringIF>,
		public MonitoringIF {
public:
	OneParameterMonitorList(uint32_t setParameterId,
			LinkedElement<MonitoringIF>* start) :
			SinglyLinkedList<MonitoringIF>(start), parameterId(
					setParameterId) {
	}

	OneParameterMonitorList() :
		SinglyLinkedList<MonitoringIF>(), parameterId(0) {
	}

	ReturnValue_t check() {
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

	ReturnValue_t setLimits(uint8_t type, const uint8_t* data, uint32_t size) {
		uint8_t position = *data;
		data++;
		size--;
		uint8_t currentPosition = 1;
		LinkedElement<MonitoringIF>* element = start;
		while ((element != NULL)) {
			if ((element->value->getLimitType() == type)) {
				if (position == currentPosition++) {
					return element->value->setLimits(type, data, size);
				}
			}
			element = element->getNext();
		}
		return MonitoringIF::MONITOR_NOT_FOUND;
	}

	ReturnValue_t setChecking(uint8_t strategy) {
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

	ReturnValue_t setToUnchecked() {
		LinkedElement<MonitoringIF>* element = start;
		ReturnValue_t result = MonitoringIF::MONITOR_NOT_FOUND;
		bool atLeastOneUpdated = false;
		while ((element != NULL)) {
			result = element->value->setToUnchecked();
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

	const uint8_t getLimitType() const {
		return LIMIT_TYPE_NO_TYPE;
	}
	const uint32_t getLimitId() const {
		return parameterId;
	}

	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerialLinkedListAdapter<MonitoringIF>::serialize(this->start,buffer,size, max_size,bigEndian);
	}

	uint32_t getSerializedSize() const {
		return SerialLinkedListAdapter<MonitoringIF>::getSerializedSize(this->start);
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerialLinkedListAdapter<MonitoringIF>::deSerialize(this->start, buffer, size, bigEndian);
	}
private:
	uint32_t parameterId;
};
#endif /* ONEPARAMETERMONITORLIST_H_ */
