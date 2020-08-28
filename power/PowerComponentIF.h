#ifndef POWERCOMPONENTIF_H_
#define POWERCOMPONENTIF_H_

#include "../serialize/SerializeIF.h"
#include "../parameters/HasParametersIF.h"

class PowerComponentIF : public SerializeIF, public HasParametersIF {
public:
	virtual ~PowerComponentIF() {

	}

	virtual object_id_t getDeviceObjectId()=0;

	virtual uint8_t getSwitchId1()=0;
	virtual uint8_t getSwitchId2()=0;
	virtual bool hasTwoSwitches()=0;

	virtual float getMin() = 0;
	virtual float getMax() = 0;

};

#endif /* POWERCOMPONENTIF_H_ */
