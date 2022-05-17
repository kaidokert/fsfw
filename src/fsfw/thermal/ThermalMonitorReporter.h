#ifndef FSFW_THERMAL_THERMALMONITORREPORTER_H_
#define FSFW_THERMAL_THERMALMONITORREPORTER_H_

#include "../monitoring/MonitorReporter.h"
#include "ThermalComponentIF.h"

/**
 * @brief   Monitor Reporter implementation for thermal components.
 */
class ThermalMonitorReporter : public MonitorReporter<float> {
 public:
  template <typename... Args>
  ThermalMonitorReporter(Args... args) : MonitorReporter<float>(std::forward<Args>(args)...) {}
  ~ThermalMonitorReporter();
  ReturnValue_t translateState(ThermalComponentIF::State state, float sample, float lowerLimit,
                               float upperLimit, bool componentIsOperational = true);

  bool isAboveHighLimit();

 protected:
  virtual void sendTransitionEvent(float currentValue, ReturnValue_t state);
};

#endif /* FSFW_THERMAL_THERMALMONITORREPORTERREPORTER_H_ */
