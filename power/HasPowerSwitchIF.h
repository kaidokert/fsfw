/**
 * @file	HasPowerSwitchIF.h
 * @brief	This file defines the HasPowerSwitchIF class.
 * @date	25.02.2014
 * @author	baetz
 */
#ifndef HASPOWERSWITCHIF_H_
#define HASPOWERSWITCHIF_H_


class HasPowerSwitchIF {
public:
	virtual ~HasPowerSwitchIF() {}
	virtual ReturnValue_t getSwitches(uint8_t *firstSwitch,
			uint8_t *secondSwitch) = 0;
	virtual void getPowerLimit(float* low, float* high) = 0;
};


#endif /* HASPOWERSWITCHIF_H_ */
