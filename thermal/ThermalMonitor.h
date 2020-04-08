#ifndef FRAMEWORK_THERMAL_THERMALMONITOR_H_
#define FRAMEWORK_THERMAL_THERMALMONITOR_H_

#include <framework/monitoring/MonitorReporter.h>
#include <framework/thermal/ThermalComponentIF.h>

/**
 * What does it do. How to use it.
 */
class ThermalMonitor: public MonitorReporter<float> {
public:
	template<typename ... Args>
	ThermalMonitor(Args ... args) :
			MonitorReporter<float>(std::forward<Args>(args)...) {
	}
	~ThermalMonitor();
	ReturnValue_t translateState(ThermalComponentIF::State state, float sample,
			float lowerLimit, float upperLimit, bool componentIsOperational = true);

	bool isAboveHighLimit();
protected:
	virtual void sendTransitionEvent(float currentValue, ReturnValue_t state);

};

#endif /* FRAMEWORK_THERMAL_THERMALMONITOR_H_ */
