/*
 * DeltaCheckMonitor.h
 *
 *  Created on: 21.07.2014
 *      Author: baetz
 */

#ifndef DELTACHECKMONITOR_H_
#define DELTACHECKMONITOR_H_

#include <framework/datapool/DataSet.h>
#include <framework/datapool/PIDReader.h>
#include <framework/monitoring/HasMonitorsIF.h>
#include <framework/monitoring/LimitViolationReporter.h>
#include <framework/monitoring/MonitoringIF.h>
#include <framework/monitoring/MonitoringMessageContent.h>
#include <framework/serialize/SerializeElement.h>



//TODO: Lots of old, unfixed stuff. Do not use!
template<typename T>
class DeltaCheckMonitor: public MonitoringIF {
public:
	DeltaCheckMonitor(HasMonitorsIF* owner, uint32_t poolId,
			T initialMinDelta, T initialMaxDelta, uint8_t arrayPos = 0,
			uint8_t initialStrategy = REPORT_ALL, Event eventToTrigger = 0) :
			owner(owner), parameter(poolId, NULL, arrayPos), oldValue(
					0), comparisonValueValid(false), minDelta(initialMinDelta), maxDelta(
					initialMaxDelta), reportingStrategy(initialStrategy), checkingState(
					CHECKING_STATUS_OK), event(eventToTrigger) {
		initialMinDelta.setNext(&initialMaxDelta);
	}
	virtual ~DeltaCheckMonitor() {

	}
	ReturnValue_t check() {
		DataSet mySet;
		mySet.registerVariable(&parameter);
		mySet.read();
		ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
		uint8_t oldState = 0;
		if (!parameter.isValid()) {
			checkingState = CHECKING_STATUS_INVALID;
			comparisonValueValid = false;
			result = PARAMETER_INVALID;
		} else {
			if (!comparisonValueValid) {
				oldValue = parameter;
				comparisonValueValid = true;
				return FIRST_SAMPLE;
			}
			oldState = checkingState;
			if ((parameter.value - oldValue) > maxDelta.entry) {
				checkingState = CHECKING_STATUS_ABOVE_HIGH_THRESHOLD;
				result = HIGH_LIMIT;
			} else if ((parameter.value - oldValue) < minDelta.entry) {
				checkingState = CHECKING_STATUS_BELOW_LOW_THRESHOLD;
				result = LOW_LIMIT;
			} else {
				checkingState = CHECKING_STATUS_OK;
			}
			if (oldState != checkingState) {
				reportViolation(oldState);
			}
		}
		if (reportingStrategy == REPORT_NONE) {
			checkingState = CHECKING_STATUS_UNSELECTED;
		}
		return result;
	}
	ReturnValue_t setLimits(uint8_t type, const uint8_t* data, uint32_t size) {
		if (type != getLimitType()) {
			return WRONG_TYPE;
		}
		UpdateLimitMonitorContent<T> content;
		int32_t tempSize = size;
		const uint8_t* pBuffer = data;
		ReturnValue_t result = content.deSerialize(&pBuffer, &tempSize, true);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		minDelta = content.lowValue.entry;
		maxDelta = content.highValue.entry;
		return HasReturnvaluesIF::RETURN_OK;
	}
	ReturnValue_t setChecking(uint8_t strategy) {
		reportingStrategy = strategy;
		return HasReturnvaluesIF::RETURN_OK;
	}
	uint32_t getPID() {
		return parameter.getParameterId();
	}
	const uint8_t getLimitType() const {
		return LIMIT_TYPE_DELTA_CHECK;
	}
private:
	HasMonitorsIF* owner;
	PIDReader<T> parameter;
	T oldValue;
	bool comparisonValueValid;
	SerializeElement<T> minDelta;
	SerializeElement<T> maxDelta;
	uint8_t reportingStrategy;
	uint8_t checkingState;
	EventId_t event;
	void reportViolation(uint8_t oldState) {
		if ((reportingStrategy & REPORT_REPORTS_ONLY) != 0) {
			if (checkingState == CHECKING_STATUS_ABOVE_HIGH_THRESHOLD) {
				MonitoringReportContent<T> report(parameter.getParameterId(),
						parameter.value, maxDelta.entry, oldState,
						checkingState);
				LimitViolationReporter::sendLimitViolationReport(&report);
			} else {
				MonitoringReportContent<T> report(parameter.getParameterId(),
						parameter.value, minDelta.entry, oldState,
						checkingState);
				LimitViolationReporter::sendLimitViolationReport(&report);
			};
		}
		if ((this->reportingStrategy & REPORT_EVENTS_ONLY) != 0) {
			if (checkingState == CHECKING_STATUS_ABOVE_HIGH_THRESHOLD || checkingState == CHECKING_STATUS_BELOW_LOW_THRESHOLD) {
				owner->forwardEvent(event, (oldState << 8) + checkingState);
			}
		}
	}
};

#endif /* DELTACHECKMONITOR_H_ */
