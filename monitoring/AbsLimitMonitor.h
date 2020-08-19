#ifndef FRAMEWORK_MONITORING_ABSLIMITMONITOR_H_
#define FRAMEWORK_MONITORING_ABSLIMITMONITOR_H_

#include "MonitorBase.h"
#include <cmath>

template<typename T>
class AbsLimitMonitor: public MonitorBase<T> {
public:
	AbsLimitMonitor(object_id_t reporterId, uint8_t monitorId, uint32_t parameterId,
			uint16_t confirmationLimit, T limit, Event violationEvent = MonitoringIF::VALUE_OUT_OF_RANGE, bool aboveIsViolation = true) :
			MonitorBase<T>(reporterId, monitorId, parameterId, confirmationLimit), limit(limit), violationEvent(violationEvent), aboveIsViolation(aboveIsViolation) {
	}
	virtual ~AbsLimitMonitor() {
	}
	virtual ReturnValue_t checkSample(T sample, T* crossedLimit) {
		*crossedLimit = limit;
		if (aboveIsViolation) {
			if ((std::abs(sample) > limit)) {
				return MonitoringIF::OUT_OF_RANGE;
			}
		} else {
			if ((std::abs(sample) < limit)) {
				return MonitoringIF::OUT_OF_RANGE;
			}
		}
		return HasReturnvaluesIF::RETURN_OK; //We're not out of range.
	}

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex) {
		ReturnValue_t result = this->MonitorBase<T>::getParameter(domainId,
				parameterId, parameterWrapper, newValues, startAtIndex);
		//We'll reuse the DOMAIN_ID of MonitorReporter, as we know the parameterIds used there.
		if (result != this->INVALID_MATRIX_ID) {
			return result;
		}
		switch (parameterId) {
		case 10:
			parameterWrapper->set(this->limit);
			break;
		default:
			return this->INVALID_MATRIX_ID;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}
	bool isOutOfLimits() {
		if (this->oldState == MonitoringIF::OUT_OF_RANGE) {
			return true;
		} else {
			return false;
		}
	}
	void setLimit(T value) {
		limit = value;
	}
protected:
	void sendTransitionEvent(T currentValue, ReturnValue_t state) {
		switch (state) {
		case MonitoringIF::OUT_OF_RANGE:
			EventManagerIF::triggerEvent(this->reportingId, violationEvent, this->parameterId);
			break;
		default:
			break;
		}
	}
	T limit;
	const Event violationEvent;
	const bool aboveIsViolation;
};

#endif /* FRAMEWORK_MONITORING_ABSLIMITMONITOR_H_ */
