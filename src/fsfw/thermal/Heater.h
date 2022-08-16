#ifndef FSFW_THERMAL_HEATER_H_
#define FSFW_THERMAL_HEATER_H_

#include <cstdint>

#include "fsfw/devicehandlers/HealthDevice.h"
#include "fsfw/parameters/ParameterHelper.h"
#include "fsfw/power/PowerSwitchIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/timemanager/Countdown.h"

class Heater : public HealthDevice, public ReceivesParameterMessagesIF {
  friend class RedundantHeater;

 public:
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::HEATER;
  static const Event HEATER_ON = MAKE_EVENT(0, severity::INFO);
  static const Event HEATER_OFF = MAKE_EVENT(1, severity::INFO);
  static const Event HEATER_TIMEOUT = MAKE_EVENT(2, severity::LOW);
  static const Event HEATER_STAYED_ON = MAKE_EVENT(3, severity::LOW);
  static const Event HEATER_STAYED_OFF = MAKE_EVENT(4, severity::LOW);

  Heater(uint32_t objectId, uint8_t switch0, uint8_t switch1);
  virtual ~Heater();

  ReturnValue_t performOperation(uint8_t opCode);

  ReturnValue_t initialize();

  ReturnValue_t set();
  void clear(bool passive);

  void setPowerSwitcher(PowerSwitchIF *powerSwitch);

  MessageQueueId_t getCommandQueue() const;

  ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId, ParameterWrapper *parameterWrapper,
                             const ParameterWrapper *newValues, uint16_t startAtIndex);

 protected:
  static const uint32_t INVALID_UPTIME = 0;

  enum InternalState {
    STATE_ON,
    STATE_OFF,
    STATE_PASSIVE,
    STATE_WAIT_FOR_SWITCHES_ON,
    STATE_WAIT_FOR_SWITCHES_OFF,
    STATE_WAIT_FOR_FDIR,  // Used to avoid doing anything until fdir decided what to do
    STATE_FAULTY,
    STATE_WAIT,  // Used when waiting for system to recover from miniops
    // Entered when under external control and a fdir reaction would be triggered.
    // This is useful when leaving external control into an unknown state
    STATE_EXTERNAL_CONTROL
    // If no fdir reaction is triggered under external control the state is still ok and
    // no need for any special treatment is needed
  } internalState;

  PowerSwitchIF *powerSwitcher = nullptr;
  MessageQueueId_t pcduQueueId = MessageQueueIF::NO_QUEUE;

  uint8_t switch0;
  uint8_t switch1;

  bool wasOn = false;

  bool timedOut = false;

  bool reactedToBeingFaulty = false;

  bool passive = false;

  MessageQueueIF *eventQueue = nullptr;
  Countdown heaterOnCountdown;
  Countdown switchCountdown;
  ParameterHelper parameterHelper;

  enum Action { SET, CLEAR } lastAction = CLEAR;

  void doAction(Action action);

  void setSwitch(uint8_t number, ReturnValue_t state, uint32_t *upTimeOfSwitching);

  void handleQueue();

  void handleEventQueue();
};

#endif /* FSFW_THERMAL_HEATER_H_ */
