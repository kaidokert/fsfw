#ifndef FSFW_MONITORING_MONITORREPORTER_H_
#define FSFW_MONITORING_MONITORREPORTER_H_

#include "../datapoollocal/localPoolDefinitions.h"
#include "../events/EventManagerIF.h"
#include "../parameters/HasParametersIF.h"
#include "LimitViolationReporter.h"
#include "MonitoringIF.h"
#include "MonitoringMessageContent.h"
#include "monitoringConf.h"

template <typename T>
class MonitorReporter : public HasParametersIF {
 public:
  static const uint8_t ENABLED = 1;
  static const uint8_t DISABLED = 0;

  // TODO: Adapt to use SID instead of parameter ID.

  MonitorReporter(object_id_t reportingId, uint8_t monitorId, gp_id_t globalPoolId,
                  uint16_t confirmationLimit)
      : monitorId(monitorId),
        globalPoolId(globalPoolId),
        reportingId(reportingId),
        oldState(MonitoringIF::UNCHECKED),
        reportingEnabled(ENABLED),
        eventEnabled(ENABLED),
        currentCounter(0),
        confirmationLimit(confirmationLimit) {}

  virtual ~MonitorReporter() {}

  ReturnValue_t monitorStateIs(ReturnValue_t state, T parameterValue = 0, T crossedLimit = 0) {
    if (state != oldState) {
      if (isConfirmed(state)) {
        if (eventEnabled == ENABLED) {
          sendTransitionEvent(parameterValue, state);
        }
        if (reportingEnabled == ENABLED) {
          sendTransitionReport(parameterValue, crossedLimit, state);
        }
        oldState = state;
      } else {
        // This is to ensure confirmation works.
        // Needs to be reset to be able to confirm against oldState again next time.
        return oldState;
      }
    } else {
      resetConfirmation();
    }
    return state;
  }

  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper *parameterWrapper,
                                     const ParameterWrapper *newValues, uint16_t startAtIndex) {
    if (domainId != monitorId) {
      return INVALID_DOMAIN_ID;
    }
    switch (uniqueId) {
      case 0:
        parameterWrapper->set(this->confirmationLimit);
        break;
      case 1:
        parameterWrapper->set(this->reportingEnabled);
        break;
      case 2:
        parameterWrapper->set(this->eventEnabled);
        break;
      default:
        return INVALID_IDENTIFIER_ID;
    }
    return returnvalue::OK;
  }
  virtual ReturnValue_t setToUnchecked() { return setToState(MonitoringIF::UNCHECKED); }
  virtual ReturnValue_t setToInvalid() { return setToState(MonitoringIF::INVALID); }
  object_id_t getReporterId() const { return reportingId; }

  void setEventEnabled(uint8_t eventEnabled) { this->eventEnabled = eventEnabled; }

  void setReportingEnabled(uint8_t reportingEnabled) { this->reportingEnabled = reportingEnabled; }

  bool isEventEnabled() const { return (eventEnabled == ENABLED); }

 protected:
  const uint8_t monitorId;
  const gp_id_t globalPoolId;
  object_id_t reportingId;
  ReturnValue_t oldState;

  uint8_t reportingEnabled;

  uint8_t eventEnabled;

  uint16_t currentCounter;
  uint16_t confirmationLimit;

  bool isConfirmed(ReturnValue_t state) {
    // Confirm INVALID and UNCHECKED immediately.
    if (state == MonitoringIF::INVALID || state == MonitoringIF::UNCHECKED) {
      currentCounter = 0;
      return true;
    }
    return doesChildConfirm(state);
  }

  /**
   * This is the most simple form of confirmation.
   * A counter counts any violation and compares the number to maxCounter.
   * @param state The state, indicating the type of violation. Not used here.
   * @return true if counter > maxCounter, else false.
   */
  virtual bool doesChildConfirm(ReturnValue_t state) {
    currentCounter += 1;
    if (currentCounter > confirmationLimit) {
      currentCounter = 0;
      return true;
    } else {
      return false;
    }
  }
  /**
   * This method needs to reset the confirmation in case a valid sample was found.
   * Here, simply resets the current counter.
   */
  virtual void resetConfirmation() { currentCounter = 0; }
  /**
   * Default version of sending transitional events.
   * Should be overridden from specialized monitors.
   * @param currentValue The current value which was monitored.
   * @param state The state the monitor changed to.
   */
  virtual void sendTransitionEvent(T currentValue, ReturnValue_t state) {
    switch (state) {
      case MonitoringIF::UNCHECKED:
      case MonitoringIF::UNSELECTED:
      case MonitoringIF::INVALID:
      case returnvalue::OK:
        break;
      default:
        EventManagerIF::triggerEvent(reportingId, MonitoringIF::MONITOR_CHANGED_STATE, state);
        break;
    }
  }
  /**
   * Default implementation for sending transition report.
   * May be overridden, but is seldom necessary.
   * @param parameterValue Current value of the parameter
   * @param crossedLimit The limit crossed (if applicable).
   * @param state Current state the monitor is in.
   */
  virtual void sendTransitionReport(T parameterValue, T crossedLimit, ReturnValue_t state) {
    MonitoringReportContent<T> report(globalPoolId, parameterValue, crossedLimit, oldState, state);
    LimitViolationReporter::sendLimitViolationReport(&report);
  }
  ReturnValue_t setToState(ReturnValue_t state) {
    if (oldState != state && reportingEnabled) {
      MonitoringReportContent<T> report(globalPoolId, 0, 0, oldState, state);
      LimitViolationReporter::sendLimitViolationReport(&report);
      oldState = state;
    }
    return returnvalue::OK;
  }
};

#endif /* FSFW_MONITORING_MONITORREPORTER_H_ */
