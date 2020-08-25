/**
 * @file	PowerSwitchIF.h
 * @brief	This file defines the PowerSwitchIF class.
 * @date	20.03.2013
 * @author	baetz
 */

#ifndef POWERSWITCHIF_H_
#define POWERSWITCHIF_H_

#include "../events/Event.h"
#include "../returnvalues/HasReturnvaluesIF.h"
/**
 * This interface defines a connection to a device that is capable of turning on and off
 * switches of devices identified by a switch ID.
 */
class PowerSwitchIF : public HasReturnvaluesIF {
public:
	/**
	 * Empty dtor.
	 */
	virtual ~PowerSwitchIF() {

	}
	/**
	 * The Returnvalues id of this class, required by HasReturnvaluesIF
	 */
	static const uint8_t INTERFACE_ID = CLASS_ID::POWER_SWITCH_IF;
	static const ReturnValue_t SWITCH_ON = MAKE_RETURN_CODE(1);
	static const ReturnValue_t SWITCH_OFF = MAKE_RETURN_CODE(0);
	static const ReturnValue_t SWITCH_TIMEOUT = MAKE_RETURN_CODE(2);
	static const ReturnValue_t FUSE_ON = MAKE_RETURN_CODE(3);
	static const ReturnValue_t FUSE_OFF = MAKE_RETURN_CODE(4);
	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PCDU_2;
	static const Event SWITCH_WENT_OFF = MAKE_EVENT(0, SEVERITY::LOW); //!< Someone detected that a switch went off which shouldn't. Severity: Low, Parameter1: switchId1, Parameter2: switchId2
	/**
	 * send a direct command to the Power Unit to enable/disable the specified switch.
	 *
	 * @param switchNr
	 * @param onOff on == @c SWITCH_ON; off != @c SWITCH_ON
	 */
	virtual void sendSwitchCommand(uint8_t switchNr, ReturnValue_t onOff) const = 0;
	/**
	 * Sends a command to the Power Unit to enable a certain fuse.
	 */
	virtual void sendFuseOnCommand(uint8_t fuseNr) const = 0;

	/**
	 * get the state of the Switches.
	 * @param switchNr
	 * @return
	 *     - @c SWITCH_ON if the specified switch is on.
	 *     - @c SWITCH_OFF if the specified switch is off.
	 *     - @c RETURN_FAILED if an error occured
	 */
	virtual ReturnValue_t getSwitchState( uint8_t switchNr ) const = 0;
	/**
	 * get state of a fuse.
	 * @param fuseNr
	 * @return
	 *     - @c FUSE_ON if the specified fuse is on.
	 *     - @c FUSE_OFF if the specified fuse is off.
	 *     - @c RETURN_FAILED if an error occured
	 */
	virtual ReturnValue_t getFuseState( uint8_t fuseNr ) const = 0;
	/**
	 * The maximum delay that it will take to change a switch
	 *
	 * This may take into account the time to send a command, wait for it to be executed and see the switch changed.
	 */
	virtual uint32_t getSwitchDelayMs(void) const = 0;
};


#endif /* POWERSWITCHIF_H_ */
