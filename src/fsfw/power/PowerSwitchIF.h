#ifndef FSFW_POWER_POWERSWITCHIF_H_
#define FSFW_POWER_POWERSWITCHIF_H_

#include "../events/Event.h"
#include "../returnvalues/returnvalue.h"
#include "definitions.h"
/**
 *
 * @brief   This interface defines a connection to a device that is capable of
 *          turning on and off switches of devices identified by a switch ID.
 * @details
 * The virtual functions of this interface do not allow to make any assignments
 * because they can be called asynchronosuly (const ending).
 * @ingroup interfaces
 */
class PowerSwitchIF {
 public:
  /**
   * Empty dtor.
   */
  virtual ~PowerSwitchIF() {}
  /**
   * The Returnvalues id of this class
   */
  static const uint8_t INTERFACE_ID = CLASS_ID::POWER_SWITCH_IF;
  static const ReturnValue_t SWITCH_ON = MAKE_RETURN_CODE(1);
  static const ReturnValue_t SWITCH_OFF = MAKE_RETURN_CODE(0);
  static const ReturnValue_t SWITCH_TIMEOUT = MAKE_RETURN_CODE(2);
  static const ReturnValue_t FUSE_ON = MAKE_RETURN_CODE(3);
  static const ReturnValue_t FUSE_OFF = MAKE_RETURN_CODE(4);
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PCDU_2;
  static const Event SWITCH_WENT_OFF = MAKE_EVENT(
      0, severity::LOW);  //!< Someone detected that a switch went off which shouldn't. Severity:
                          //!< Low, Parameter1: switchId1, Parameter2: switchId2
  /**
   * send a direct command to the Power Unit to enable/disable the specified switch.
   *
   * @param switchNr
   * @param onOff on == @c SWITCH_ON; off != @c SWITCH_ON
   */
  virtual ReturnValue_t sendSwitchCommand(power::Switch_t switchNr, ReturnValue_t onOff) = 0;
  /**
   * Sends a command to the Power Unit to enable a certain fuse.
   */
  virtual ReturnValue_t sendFuseOnCommand(uint8_t fuseNr) = 0;

  /**
   * get the state of the Switches.
   * @param switchNr
   * @return
   *     - @c SWITCH_ON if the specified switch is on.
   *     - @c SWITCH_OFF if the specified switch is off.
   *     - @c returnvalue::FAILED if an error occured
   */
  virtual ReturnValue_t getSwitchState(power::Switch_t switchNr) const = 0;
  /**
   * get state of a fuse.
   * @param fuseNr
   * @return
   *     - @c FUSE_ON if the specified fuse is on.
   *     - @c FUSE_OFF if the specified fuse is off.
   *     - @c returnvalue::FAILED if an error occured
   */
  virtual ReturnValue_t getFuseState(uint8_t fuseNr) const = 0;
  /**
   * The maximum delay that it will take to change a switch
   *
   * This may take into account the time to send a command, wait for it to be executed and see the
   * switch changed.
   */
  virtual uint32_t getSwitchDelayMs(void) const = 0;
};

#endif /* FSFW_POWER_POWERSWITCHIF_H_ */
