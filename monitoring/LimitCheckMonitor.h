/*
 * LimitCheckMonitor.h
 *
 *  Created on: 21.07.2014
 *      Author: baetz
 */

#ifndef LIMITCHECKMONITOR_H_
#define LIMITCHECKMONITOR_H_

#include <framework/container/LinkedElementDecorator.h>
#include <framework/datapool/DataSet.h>
#include <framework/datapool/PIDReader.h>
#include <framework/monitoring/LimitViolationReporter.h>
#include <framework/monitoring/MonitoringMessageContent.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/events/EventManagerIF.h>

template<typename T>
class LimitCheckMonitor: public MonitoringIF {
public:
	struct CheckParameters {

		CheckParameters() :
				oldState(HasReturnvaluesIF::RETURN_OK), parameterId(
						PoolVariableIF::NO_PARAMETER), sendTransitionreport(
						false), sendTransitionEvent(false), lowerTransitionEvent(
						0), upperTransitionEvent(0), reportingObject(0), currentCounter(
						0), maxCounter(0) {
		}

		CheckParameters(ReturnValue_t oldState, const uint32_t parameterId,
				bool sendTransitionreport, bool sendTransitionEvent,
				Event lowerTransitionEvent, Event upperTransitionEvent,
				const object_id_t reportingObject, uint16_t maxCounter = 0) :
				oldState(oldState), parameterId(parameterId), sendTransitionreport(
						sendTransitionreport), sendTransitionEvent(
						sendTransitionEvent), lowerTransitionEvent(
						lowerTransitionEvent), upperTransitionEvent(
						upperTransitionEvent), reportingObject(reportingObject), currentCounter(
						0), maxCounter(maxCounter) {
		}

		ReturnValue_t oldState;
		const uint32_t parameterId;
		bool sendTransitionreport;
		bool sendTransitionEvent;
		Event lowerTransitionEvent;
		Event upperTransitionEvent;
		const object_id_t reportingObject;
		uint16_t currentCounter;
		uint16_t maxCounter;
	};

	LimitCheckMonitor(object_id_t reportAs, uint32_t parameterId,
			T initialLower, T initialUpper, bool generateReports = true,
			bool throwEvents = false, Event lowerTransitionEvent = 0,
			Event upperTransitionEvent = 0, uint16_t confirmationNumber = 0) :
			lowerLimit(initialLower), upperLimit(initialUpper), parameters(
					UNCHECKED, parameterId, generateReports, throwEvents,
					lowerTransitionEvent, upperTransitionEvent, reportAs,
					confirmationNumber) {
		lowerLimit.setNext(&upperLimit);
	}

	virtual ~LimitCheckMonitor() {
	}

	virtual ReturnValue_t check() {
		DataSet mySet;
		PIDReader<T> parameter(parameters.parameterId, &mySet);
		mySet.read();
		if (!parameter.isValid()) {
			if (parameters.oldState != INVALID) {
				MonitoringReportContent<T> report(parameter.getParameterId(), 0,
						0, parameters.oldState, INVALID);
				LimitViolationReporter::sendLimitViolationReport(&report);
				parameters.oldState = INVALID;
			}
		} else {
			parameters.oldState = doCheck(parameter.value, lowerLimit,
					upperLimit, &parameters);
		}
		return parameters.oldState;
	}

	static ReturnValue_t doCheck(T value, T lowerLimit, T upperLimit,
			CheckParameters* checkParameters, ReturnValue_t lowerReturnCode =
					BELOW_LOW_LIMIT, ReturnValue_t upperReturnCode =
					ABOVE_HIGH_LIMIT) {
		return doCheck(value, lowerLimit, upperLimit, checkParameters->oldState,
				checkParameters->parameterId,
				checkParameters->sendTransitionreport,
				checkParameters->sendTransitionEvent,
				checkParameters->lowerTransitionEvent,
				checkParameters->upperTransitionEvent,
				checkParameters->reportingObject,
				&checkParameters->currentCounter, checkParameters->maxCounter,
				lowerReturnCode, upperReturnCode);
	}

	static ReturnValue_t doCheck(T value, T lowerLimit, T upperLimit,
			ReturnValue_t oldState, uint32_t parameterId,
			bool sendTransitionreport, bool sendTransitionEvent,
			Event lowerTransitionEvent, Event upperTransitionEvent,
			object_id_t reportingObject, uint16_t *currentCounter = NULL,
			uint16_t maxCounter = 0, ReturnValue_t lowerReturnCode =
					BELOW_LOW_LIMIT, ReturnValue_t upperReturnCode =
					ABOVE_HIGH_LIMIT) {
		uint16_t tempCounter = 0;
		if (currentCounter == NULL) {
			currentCounter = &tempCounter;
			maxCounter = 0;
		}

		ReturnValue_t currentState = HasReturnvaluesIF::RETURN_OK;
		T crossedLimit = 0;
		if (value > upperLimit) {
			currentState = upperReturnCode;
			crossedLimit = upperLimit;
		} else if (value < lowerLimit) {
			currentState = lowerReturnCode;
			crossedLimit = lowerLimit;
		}

		if (oldState != currentState) {
			//confirmation
			*currentCounter += 1;
			if (*currentCounter > maxCounter  || oldState == UNCHECKED) {
				*currentCounter = 0;
				// 	Distinction of 3 cases: up/down/(ok or default)
				if (currentState == upperReturnCode) {
					if (sendTransitionEvent) {
						EventManagerIF::triggerEvent(reportingObject,
								upperTransitionEvent, 0,
								0);
					}
				} else if (currentState == lowerReturnCode) {
					if (sendTransitionEvent) {
						EventManagerIF::triggerEvent(reportingObject,
								lowerTransitionEvent, 0,
								0);
					}
				} else {
					// RETURN_OK or all other cases
					if (oldState == lowerReturnCode) {
						crossedLimit = lowerLimit;
					} else {
						crossedLimit = upperLimit;
					}
				}

				if (sendTransitionreport) {
					MonitoringReportContent<T> report(parameterId, value,
							crossedLimit, oldState, currentState);
					LimitViolationReporter::sendLimitViolationReport(&report);
				} else {
					currentState = UNSELECTED;
				}
			} else {
				currentState = oldState;
			}
		} else {
			*currentCounter = 0;
		}
		return currentState;
	}

	ReturnValue_t setLimits(uint8_t type, const uint8_t* data, uint32_t size) {
		if (type != getLimitType()) {
			return WRONG_TYPE;
		}

		UpdateLimitMonitorContent<T> content;

		if (size != content.getSerializedSize()) {
			return INVALID_SIZE;
		}

		int32_t tempSize = size;
		const uint8_t* pBuffer = data;
		ReturnValue_t result = content.deSerialize(&pBuffer, &tempSize, true);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		setLimits(content.lowValue, content.highValue);
		return HasReturnvaluesIF::RETURN_OK;
	}

	void setLimits(T lower, T upper) {
		lowerLimit = lower;
		upperLimit = upper;
	}

	ReturnValue_t setChecking(uint8_t strategy) {
		if ((strategy & REPORT_EVENTS_ONLY) != 0) {
			parameters.sendTransitionEvent = true;
		} else {
			parameters.sendTransitionEvent = false;
		}
		if ((strategy & REPORT_REPORTS_ONLY) != 0) {
			parameters.sendTransitionreport = true;
		} else {
			parameters.sendTransitionreport = false;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t setToUnchecked() {
		return setToState(UNCHECKED, &parameters);
	}

	static ReturnValue_t setToUnchecked(CheckParameters* parameters) {
		if (parameters->oldState != UNCHECKED) {
			MonitoringReportContent<float> report(parameters->parameterId, 0, 0,
					parameters->oldState, UNCHECKED);
			LimitViolationReporter::sendLimitViolationReport(&report);
			parameters->oldState = UNCHECKED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	static ReturnValue_t setToState(ReturnValue_t newState, CheckParameters* parameters) {
		if (parameters->oldState != newState) {
			MonitoringReportContent<float> report(parameters->parameterId, 0, 0,
					parameters->oldState, newState);
			LimitViolationReporter::sendLimitViolationReport(&report);
			parameters->oldState = newState;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerialLinkedListAdapter<SerializeIF>::serialize(&lowerLimit,
				buffer, size, max_size, bigEndian);
	}

	uint32_t getSerializedSize() const {
		return SerialLinkedListAdapter<SerializeIF>::getSerializedSize(
				&lowerLimit);
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
	bool bigEndian) {
		return SerialLinkedListAdapter<SerializeIF>::deSerialize(&lowerLimit,
				buffer, size, bigEndian);
	}
	const uint8_t getLimitType() const {
		return LIMIT_TYPE_LIMIT_CHECK;
	}
	const uint32_t getLimitId() const {
		return parameters.parameterId;
	}
	T getLowerLimit() {
		return lowerLimit;
	}
	T getUpperLimit() {
		return upperLimit;
	}
private:
	SerializeElement<T> lowerLimit;
	SerializeElement<T> upperLimit;
	CheckParameters parameters;
};

//TODO: This is for float only, as this is currently the only need.
typedef LinkedElementDecorator<LimitCheckMonitor<float>, MonitoringIF> LinkedLimitMonitor;

#endif /* LIMITCHECKMONITOR_H_ */
