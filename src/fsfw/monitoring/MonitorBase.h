#ifndef FSFW_MONITORING_MONITORBASE_H_
#define FSFW_MONITORING_MONITORBASE_H_

#include "../datapoollocal/LocalPoolVariable.h"
#include "LimitViolationReporter.h"
#include "MonitorReporter.h"
#include "MonitoringIF.h"
#include "MonitoringMessageContent.h"
#include "monitoringConf.h"

/**
 * @brief   Base class for monitoring of parameters.
 * @details
 * Can be used anywhere, specializations need to implement checkSample and
 * should override sendTransitionEvent.
 * Manages state handling, enabling and disabling of events/reports and
 * forwarding of transition reports via MonitorReporter.
 *
 * In addition, it provides default implementations for fetching the
 * parameter sample from the data pool and a simple confirmation counter.
 */
template <typename T>
class MonitorBase : public MonitorReporter<T> {
 public:
  MonitorBase(object_id_t reporterId, uint8_t monitorId, gp_id_t globalPoolId,
              uint16_t confirmationLimit)
      : MonitorReporter<T>(reporterId, monitorId, globalPoolId, confirmationLimit),
        poolVariable(globalPoolId) {}

  virtual ~MonitorBase() {}

  virtual ReturnValue_t check() {
    // 1. Fetch sample of type T, return validity.
    T sample = 0;
    ReturnValue_t validity = fetchSample(&sample);

    // 2. If returning from fetch != OK, parameter is invalid.
    // Report (if oldState is != invalidity).
    if (validity != returnvalue::OK) {
      this->monitorStateIs(validity, sample, 0);
    } else {
      // 3. Otherwise, check sample.
      this->oldState = doCheck(sample);
    }
    return this->oldState;
  }
  virtual ReturnValue_t doCheck(T sample) {
    T crossedLimit = 0.0;
    ReturnValue_t currentState = checkSample(sample, &crossedLimit);
    return this->monitorStateIs(currentState, sample, crossedLimit);
  }

  // Abstract or default.
  virtual ReturnValue_t checkSample(T sample, T* crossedLimit) = 0;

 protected:
  virtual ReturnValue_t fetchSample(T* sample) {
    ReturnValue_t result = poolVariable.read();
    if (result != returnvalue::OK) {
      return result;
    }
    if (not poolVariable.isValid()) {
      return MonitoringIF::INVALID;
    }
    *sample = poolVariable.value;
    return returnvalue::OK;
  }

  LocalPoolVariable<T> poolVariable;
};

#endif /* FSFW_MONITORING_MONITORBASE_H_ */
