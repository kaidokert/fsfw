#ifndef FSFW_DEVICEHANDLERS_DEVICEHANDLERFAILUREISOLATION_H_
#define FSFW_DEVICEHANDLERS_DEVICEHANDLERFAILUREISOLATION_H_

#include "../fdir/FailureIsolationBase.h"
#include "../fdir/FaultCounter.h"

namespace Factory {
void setStaticFrameworkObjectIds();
}

class DeviceHandlerFailureIsolation : public FailureIsolationBase {
  friend void(Factory::setStaticFrameworkObjectIds)();
  friend class Heater;

 public:
  DeviceHandlerFailureIsolation(object_id_t owner, object_id_t parent);
  ~DeviceHandlerFailureIsolation();
  ReturnValue_t initialize();
  void triggerEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
  bool isFdirActionInProgress();
  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper* parameterWrapper,
                                     const ParameterWrapper* newValues, uint16_t startAtIndex);

 protected:
  FaultCounter strangeReplyCount;
  FaultCounter missedReplyCount;
  FaultCounter recoveryCounter;

  enum FDIRState { NONE, RECOVERY_ONGOING, DEVICE_MIGHT_BE_OFF, AWAIT_SHUTDOWN };
  FDIRState fdirState;

  MessageQueueId_t powerConfirmation = MessageQueueIF::NO_QUEUE;
  static object_id_t powerConfirmationId;

  static const uint32_t DEFAULT_MAX_REBOOT = 1;
  static const uint32_t DEFAULT_REBOOT_TIME_MS = 180000;
  static const uint32_t DEFAULT_MAX_STRANGE_REPLIES = 10;
  static const uint32_t DEFAULT_STRANGE_REPLIES_TIME_MS = 10000;
  static const uint32_t DEFAULT_MAX_MISSED_REPLY_COUNT = 5;
  static const uint32_t DEFAULT_MISSED_REPLY_TIME_MS = 10000;

  virtual ReturnValue_t eventReceived(EventMessage* event);
  virtual void eventConfirmed(EventMessage* event);
  void wasParentsFault(EventMessage* event);
  void decrementFaultCounters();
  void handleRecovery(Event reason);
  virtual void clearFaultCounters();
  void setFdirState(FDIRState state);
  void startRecovery(Event reason);
  void setFaulty(Event reason);

  bool isFdirInActionOrAreWeFaulty(EventMessage* event);
};

#endif /* FSFW_DEVICEHANDLERS_DEVICEHANDLERFAILUREISOLATION_H_ */
