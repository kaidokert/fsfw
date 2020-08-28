#ifndef FRAMEWORK_FDIR_FAULTCOUNTER_H_
#define FRAMEWORK_FDIR_FAULTCOUNTER_H_

#include "../parameters/HasParametersIF.h"
#include "../timemanager/Countdown.h"

class FaultCounter: public HasParametersIF {
public:
	FaultCounter();
	FaultCounter(uint32_t failureThreshold, uint32_t decrementAfterMs,
			uint8_t setParameterDomain = 0);
	virtual ~FaultCounter();

	bool incrementAndCheck(uint32_t amount = 1);

	void increment(uint32_t amount = 1);

	bool checkForDecrement();

	bool aboveThreshold();

	void clear();
	void setFailureThreshold(uint32_t failureThreshold);
	void setFaultDecrementTimeMs(uint32_t timeMs);

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);

	void setParameterDomain(uint8_t domain);
private:
	uint8_t parameterDomain;
	Countdown timer;
	uint32_t faultCount;
	uint32_t failureThreshold;
};

#endif /* FRAMEWORK_FDIR_FAULTCOUNTER_H_ */
