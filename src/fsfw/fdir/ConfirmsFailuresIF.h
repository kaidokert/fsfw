#ifndef FRAMEWORK_FDIR_CONFIRMSFAILURESIF_H_
#define FRAMEWORK_FDIR_CONFIRMSFAILURESIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "../returnvalues/returnvalue.h"

class ConfirmsFailuresIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::HANDLES_FAILURES_IF;
  static const ReturnValue_t YOUR_FAULT = MAKE_RETURN_CODE(0);
  static const ReturnValue_t MY_FAULT = MAKE_RETURN_CODE(1);
  static const ReturnValue_t CONFIRM_LATER = MAKE_RETURN_CODE(2);
  virtual ~ConfirmsFailuresIF() {}
  virtual MessageQueueId_t getEventReceptionQueue() = 0;
};

#endif /* FRAMEWORK_FDIR_CONFIRMSFAILURESIF_H_ */
