#ifndef FRAMEWORK_IPC_FWMESSAGETYPES_H_
#define FRAMEWORK_IPC_FWMESSAGETYPES_H_

namespace messagetypes {
// Remember to add new Message Types to the clearCommandMessage function!
enum FsfwMessageTypes {
  COMMAND = 0,
  MODE_COMMAND,
  HEALTH_COMMAND,
  MODE_SEQUENCE,
  ACTION,
  CFDP,
  TM_STORE,
  DEVICE_HANDLER_COMMAND,
  MONITORING,
  MEMORY,
  PARAMETER,
  FILE_SYSTEM_MESSAGE,
  HOUSEKEEPING,

  FW_MESSAGES_COUNT,
};
}  // namespace messagetypes

#endif /* FRAMEWORK_IPC_FWMESSAGETYPES_H_ */
