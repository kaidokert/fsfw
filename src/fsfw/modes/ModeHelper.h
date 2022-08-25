#ifndef FSFW_MODES_MODEHELPER_H_
#define FSFW_MODES_MODEHELPER_H_

#include "ModeMessage.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/timemanager/Countdown.h"

class HasModesIF;

class ModeHelper {
 public:
  MessageQueueId_t theOneWhoCommandedAMode = MessageQueueIF::NO_QUEUE;
  Mode_t commandedMode;
  Submode_t commandedSubmode;

  ModeHelper(HasModesIF *owner);
  virtual ~ModeHelper();

  ReturnValue_t handleModeCommand(CommandMessage *message);

  /**
   * @param parentQueue the Queue id of the parent object.
   * Set to 0 if no parent present
   */
  void setParentQueue(MessageQueueId_t parentQueueId);

  ReturnValue_t initialize(MessageQueueId_t parentQueueId);

  ReturnValue_t initialize(void);

  void modeChanged(Mode_t mode, Submode_t submode);

  void startTimer(uint32_t timeoutMs);

  bool isTimedOut();

  bool isForced();

  void setForced(bool forced);

  #ifdef FSFW_INTROSPECTION
  std::vector<std::pair<Mode_t, const char *>> getModes() const;

  std::vector<std::pair<Submode_t, const char *>> getSubmodes(Mode_t mode) const;
#endif

 protected:
  HasModesIF *owner;
  MessageQueueId_t parentQueueId = MessageQueueIF::NO_QUEUE;

  Countdown countdown;

  bool forced;

 private:
  void sendModeReplyMessage(Mode_t ownerMode, Submode_t ownerSubmode);
  void sendModeInfoMessage(Mode_t ownerMode, Submode_t ownerSubmode);
};

#endif /* FSFW_MODES_MODEHELPER_H_ */
