#include "fsfw/thermal/ThermalMonitorReporter.h"

#include "fsfw/monitoring/MonitoringIF.h"
#include "fsfw/thermal/ThermalComponentIF.h"

ThermalMonitorReporter::~ThermalMonitorReporter() {}

void ThermalMonitorReporter::sendTransitionEvent(float currentValue, ReturnValue_t state) {
  switch (state) {
    case MonitoringIF::BELOW_LOW_LIMIT:
      EventManagerIF::triggerEvent(reportingId, ThermalComponentIF::COMPONENT_TEMP_OOL_LOW, state);
      break;
    case MonitoringIF::ABOVE_HIGH_LIMIT:
      EventManagerIF::triggerEvent(reportingId, ThermalComponentIF::COMPONENT_TEMP_OOL_HIGH, state);
      break;
    case ThermalComponentIF::BELOW_OPERATIONAL_LIMIT:
      EventManagerIF::triggerEvent(reportingId, ThermalComponentIF::COMPONENT_TEMP_LOW, state);
      break;
    case ThermalComponentIF::ABOVE_OPERATIONAL_LIMIT:
      EventManagerIF::triggerEvent(reportingId, ThermalComponentIF::COMPONENT_TEMP_HIGH, state);
      break;
    default:
      break;
  }
}

bool ThermalMonitorReporter::isAboveHighLimit() {
  if (oldState == ThermalComponentIF::ABOVE_OPERATIONAL_LIMIT) {
    return true;
  } else {
    return false;
  }
}

ReturnValue_t ThermalMonitorReporter::translateState(ThermalComponentIF::State state, float sample,
                                                     float lowerLimit, float upperLimit,
                                                     bool componentIsOperational) {
  if (ThermalComponentIF::isIgnoredState(state)) {
    setToUnchecked();
    return MonitoringIF::UNCHECKED;
  }
  switch (state) {
    case ThermalComponentIF::OUT_OF_RANGE_LOW:
      return monitorStateIs(MonitoringIF::BELOW_LOW_LIMIT, sample, lowerLimit);
    case ThermalComponentIF::NON_OPERATIONAL_LOW:
      if (componentIsOperational) {
        return monitorStateIs(ThermalComponentIF::BELOW_OPERATIONAL_LIMIT, sample, lowerLimit);
      } else {
        return monitorStateIs(returnvalue::OK, sample, 0.0);
      }
    case ThermalComponentIF::OPERATIONAL:
      return monitorStateIs(returnvalue::OK, sample, 0.0);
    case ThermalComponentIF::NON_OPERATIONAL_HIGH:
      if (componentIsOperational) {
        return monitorStateIs(ThermalComponentIF::ABOVE_OPERATIONAL_LIMIT, sample, upperLimit);
      } else {
        return monitorStateIs(returnvalue::OK, sample, 0.0);
      }
    case ThermalComponentIF::OUT_OF_RANGE_HIGH:
      return monitorStateIs(MonitoringIF::ABOVE_HIGH_LIMIT, sample, upperLimit);
    default:
      // Never reached, all states covered.
      return returnvalue::FAILED;
  }
}
