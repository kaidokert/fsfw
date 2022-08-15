#ifndef FRAMEWORK_MONITORING_TWOVALUELIMITMONITOR_H_
#define FRAMEWORK_MONITORING_TWOVALUELIMITMONITOR_H_

#include "LimitMonitor.h"
#include "monitoringConf.h"

template <typename T>
class TwoValueLimitMonitor : public LimitMonitor<T> {
 public:
  TwoValueLimitMonitor(object_id_t reporterId, uint8_t monitorId, uint32_t lowParameterId,
                       uint32_t highParameterId, uint16_t confirmationLimit, T lowerLimit,
                       T upperLimit, Event belowLowEvent = MonitoringIF::VALUE_BELOW_LOW_LIMIT,
                       Event aboveHighEvent = MonitoringIF::VALUE_ABOVE_HIGH_LIMIT)
      : LimitMonitor<T>(reporterId, monitorId, lowParameterId, confirmationLimit, lowerLimit,
                        upperLimit, belowLowEvent, aboveHighEvent),
        highValueParameterId(highParameterId) {}
  virtual ~TwoValueLimitMonitor() {}
  ReturnValue_t doCheck(T lowSample, T highSample) {
    T crossedLimit;
    ReturnValue_t currentState = this->checkSample(lowSample, &crossedLimit);
    if (currentState != returnvalue::OK) {
      return this->monitorStateIs(currentState, lowSample, crossedLimit);
    }
    currentState = this->checkSample(highSample, &crossedLimit);
    return this->monitorStateIs(currentState, highSample, crossedLimit);
  }

 protected:
  virtual void sendTransitionReport(T parameterValue, T crossedLimit, ReturnValue_t state) {
    uint32_t usedParameterId = this->parameterId;
    if (state == MonitoringIF::ABOVE_HIGH_LIMIT) {
      usedParameterId = this->highValueParameterId;
    }
    MonitoringReportContent<T> report(usedParameterId, parameterValue, crossedLimit, this->oldState,
                                      state);
    LimitViolationReporter::sendLimitViolationReport(&report);
  }

 private:
  const uint32_t highValueParameterId;
};

#endif /* FRAMEWORK_MONITORING_TWOVALUELIMITMONITOR_H_ */
