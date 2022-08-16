#ifndef THERMALCOMPONENTIF_H_
#define THERMALCOMPONENTIF_H_

#include "../events/Event.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../parameters/HasParametersIF.h"
#include "../returnvalues/returnvalue.h"

class ThermalComponentIF : public HasParametersIF {
 public:
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::TCS_1;
  static const Event COMPONENT_TEMP_LOW = MAKE_EVENT(1, severity::LOW);
  static const Event COMPONENT_TEMP_HIGH = MAKE_EVENT(2, severity::LOW);
  static const Event COMPONENT_TEMP_OOL_LOW = MAKE_EVENT(3, severity::LOW);
  static const Event COMPONENT_TEMP_OOL_HIGH = MAKE_EVENT(4, severity::LOW);
  static const Event TEMP_NOT_IN_OP_RANGE = MAKE_EVENT(
      5, severity::LOW);  //!< Is thrown when a device should start-up, but the temperature is out
                          //!< of OP range. P1: thermalState of the component, P2: 0

  static const uint8_t INTERFACE_ID = CLASS_ID::THERMAL_COMPONENT_IF;
  static const ReturnValue_t INVALID_TARGET_STATE = MAKE_RETURN_CODE(1);
  static const ReturnValue_t ABOVE_OPERATIONAL_LIMIT = MAKE_RETURN_CODE(0xF1);
  static const ReturnValue_t BELOW_OPERATIONAL_LIMIT = MAKE_RETURN_CODE(0xF2);

  enum State {
    OUT_OF_RANGE_LOW = -2,
    NON_OPERATIONAL_LOW = -1,
    OPERATIONAL = 0,
    NON_OPERATIONAL_HIGH = 1,
    OUT_OF_RANGE_HIGH = 2,
    OUT_OF_RANGE_LOW_IGNORED = OUT_OF_RANGE_LOW - 10,
    NON_OPERATIONAL_LOW_IGNORED = NON_OPERATIONAL_LOW - 10,
    OPERATIONAL_IGNORED = OPERATIONAL + 10,
    NON_OPERATIONAL_HIGH_IGNORED = NON_OPERATIONAL_HIGH + 10,
    OUT_OF_RANGE_HIGH_IGNORED = OUT_OF_RANGE_HIGH + 10,
    UNKNOWN = 20
  };

  enum StateRequest {
    STATE_REQUEST_HEATING = 4,
    STATE_REQUEST_IGNORE = 3,
    STATE_REQUEST_OPERATIONAL = 1,
    STATE_REQUEST_NON_OPERATIONAL = 0
  };

  /**
   * The elements are ordered by priority, lowest have highest priority
   */
  enum Priority {
    SAFE = 0,             //!< SAFE
    IDLE,                 //!< IDLE
    PAYLOAD,              //!< PAYLOAD
    NUMBER_OF_PRIORITIES  //!< MAX_PRIORITY
  };

  /**
   * The elements are ordered by priority, lowest have highest priority
   */
  enum HeaterRequest {
    HEATER_REQUEST_EMERGENCY_OFF = 0,  //!< REQUEST_EMERGENCY_OFF
    HEATER_REQUEST_EMERGENCY_ON,       //!< REQUEST_EMERGENCY_ON
    HEATER_REQUEST_OFF,                //!< REQUEST_OFF
    HEATER_REQUEST_ON,                 //!< REQUEST_ON
    HEATER_DONT_CARE                   //!< DONT_CARE
  };

  virtual ~ThermalComponentIF() {}

  virtual HeaterRequest performOperation(uint8_t opCode) = 0;

  virtual object_id_t getObjectId() = 0;

  virtual uint8_t getDomainId() const = 0;

  virtual void markStateIgnored() = 0;

  virtual float getLowerOpLimit() = 0;

  virtual ReturnValue_t setTargetState(int8_t state) = 0;

  virtual void setOutputInvalid() = 0;

  static bool isOperational(int8_t state) {
    return ((state == OPERATIONAL) || (state == OPERATIONAL_IGNORED));
  }

  static bool isOutOfRange(State state) {
    return ((state == OUT_OF_RANGE_HIGH) || (state == OUT_OF_RANGE_HIGH_IGNORED) ||
            (state == OUT_OF_RANGE_LOW) || (state == OUT_OF_RANGE_LOW_IGNORED) ||
            (state == UNKNOWN));
  }

  static bool isNonOperational(State state) { return !isOutOfRange(state); }

  static bool isIgnoredState(State state) {
    switch (state) {
      case OUT_OF_RANGE_LOW_IGNORED:
      case OUT_OF_RANGE_HIGH_IGNORED:
      case NON_OPERATIONAL_LOW_IGNORED:
      case NON_OPERATIONAL_HIGH_IGNORED:
      case OPERATIONAL_IGNORED:
      case UNKNOWN:
        return true;
      default:
        return false;
    }
  }
};

#endif /* THERMALCOMPONENTIF_H_ */
