#ifndef MONITORBASE_H_
#define MONITORBASE_H_

#include "../datapool/DataSet.h"
#include "../datapool/PIDReader.h"
#include "LimitViolationReporter.h"
#include "MonitoringIF.h"
#include "MonitoringMessageContent.h"
#include "MonitorReporter.h"

/**
 * Base class for monitoring of parameters.
 * Can be used anywhere, specializations need to implement checkSample and should override sendTransitionEvent.
 * Manages state handling, enabling and disabling of events/reports and forwarding of transition
 * reports via MonitorReporter. In addition, it provides default implementations for fetching the parameter sample from
 * the data pool and a simple confirmation counter.
 */
template<typename T>
class MonitorBase: public MonitorReporter<T> {
public:
	MonitorBase(object_id_t reporterId, uint8_t monitorId,
			uint32_t parameterId, uint16_t confirmationLimit) :
			MonitorReporter<T>(reporterId, monitorId, parameterId, confirmationLimit) {
	}
	virtual ~MonitorBase() {
	}
	virtual ReturnValue_t check() {
		//1. Fetch sample of type T, return validity.
		T sample = 0;
		ReturnValue_t validity = fetchSample(&sample);

		//2. If returning from fetch != OK, parameter is invalid. Report (if oldState is != invalidity).
		if (validity != HasReturnvaluesIF::RETURN_OK) {
			this->monitorStateIs(validity, sample, 0);
			//3. Otherwise, check sample.
		} else {
			this->oldState = doCheck(sample);
		}
		return this->oldState;
	}
	virtual ReturnValue_t doCheck(T sample) {
		T crossedLimit = 0.0;
		ReturnValue_t currentState = checkSample(sample, &crossedLimit);
		return this->monitorStateIs(currentState,sample, crossedLimit);
	}
	//Abstract or default.
	virtual ReturnValue_t checkSample(T sample, T* crossedLimit) = 0;

protected:
	virtual ReturnValue_t fetchSample(T* sample) {
		DataSet mySet;
		PIDReader<T> parameter(this->parameterId, &mySet);
		mySet.read();
		if (!parameter.isValid()) {
			return MonitoringIF::INVALID;
		}
		*sample = parameter.value;
		return HasReturnvaluesIF::RETURN_OK;
	}
};

#endif /* MONITORBASE_H_ */
