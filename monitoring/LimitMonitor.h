#ifndef FRAMEWORK_MONITORING_LIMITMONITOR_H_
#define FRAMEWORK_MONITORING_LIMITMONITOR_H_

#include "MonitorBase.h"

/**
 * Variant of a limit checking class.
 * Newer version as compared to LimitCheckMonitor.
 * Functionality is more or less the same, but does not use
 * heavy weight MonitoringIF.
 */
template<typename T>
class LimitMonitor: public MonitorBase<T> {
public:
	LimitMonitor(object_id_t reporterId, uint8_t monitorId, uint32_t parameterId,
			uint16_t confirmationLimit, T lowerLimit, T upperLimit,
			Event belowLowEvent = MonitoringIF::VALUE_BELOW_LOW_LIMIT,
			Event aboveHighEvent = MonitoringIF::VALUE_ABOVE_HIGH_LIMIT) :
			MonitorBase<T>(reporterId, monitorId, parameterId, confirmationLimit), lowerLimit(
					lowerLimit), upperLimit(upperLimit), belowLowEvent(
					belowLowEvent), aboveHighEvent(aboveHighEvent) {
	}
	virtual ~LimitMonitor() {
	}
	virtual ReturnValue_t checkSample(T sample, T* crossedLimit) {
		*crossedLimit = 0.0;
		if (sample > upperLimit) {
			*crossedLimit = upperLimit;
			return MonitoringIF::ABOVE_HIGH_LIMIT;
		} else if (sample < lowerLimit) {
			*crossedLimit = lowerLimit;
			return MonitoringIF::BELOW_LOW_LIMIT;
		} else {
			return HasReturnvaluesIF::RETURN_OK; //Within limits.
		}
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
			parameterWrapper->set(this->lowerLimit);
			break;
		case 11:
			parameterWrapper->set(this->upperLimit);
			break;
		default:
			return this->INVALID_MATRIX_ID;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}
	bool isOutOfLimits() {
		if (this->oldState == MonitoringIF::ABOVE_HIGH_LIMIT || this->oldState == MonitoringIF::BELOW_LOW_LIMIT) {
			return true;
		} else {
			return false;
		}
	}

	T getLowerLimit() const {
		return lowerLimit;
	}

	T getUpperLimit() const {
		return upperLimit;
	}

protected:
	void sendTransitionEvent(T currentValue, ReturnValue_t state) {
		switch (state) {
		case MonitoringIF::BELOW_LOW_LIMIT:
			EventManagerIF::triggerEvent(this->reportingId, belowLowEvent, this->parameterId);
			break;
		case MonitoringIF::ABOVE_HIGH_LIMIT:
			EventManagerIF::triggerEvent(this->reportingId, aboveHighEvent, this->parameterId);
			break;
		default:
			break;
		}
	}
	T lowerLimit;
	T upperLimit;
	const Event belowLowEvent;
	const Event aboveHighEvent;
};

#endif /* FRAMEWORK_MONITORING_LIMITMONITOR_H_ */
