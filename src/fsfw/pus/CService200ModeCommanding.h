#ifndef FSFW_PUS_CSERVICE200MODECOMMANDING_H_
#define FSFW_PUS_CSERVICE200MODECOMMANDING_H_

#include "fsfw/tmtcservices/CommandingServiceBase.h"

/**
 * @brief Custom PUS service to set mode of all objects implementing HasModesIF
 *
 * Examples: Device Handlers, Assemblies or Subsystems.
 * Full Documentation: ECSS-E-ST-70-41C or ECSS-E-70-41A
 * Dissertation Baetz p. 115, 116, 165-167.
 *
 * This is a gateway service. It relays device commands using the software bus.
 * @ingroup pus_services
 */
class CService200ModeCommanding : public CommandingServiceBase {
 public:
  CService200ModeCommanding(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                            uint8_t numParallelCommands = 4, uint16_t commandTimeoutSeconds = 60);
  virtual ~CService200ModeCommanding();

 protected:
  //! CommandingServiceBase (CSB) abstract functions. See CSB documentation.
  ReturnValue_t isValidSubservice(uint8_t subservice) override;
  ReturnValue_t getMessageQueueAndObject(uint8_t subservice, const uint8_t *tcData,
                                         size_t tcDataLen, MessageQueueId_t *id,
                                         object_id_t *objectId) override;
  ReturnValue_t prepareCommand(CommandMessage *message, uint8_t subservice, const uint8_t *tcData,
                               size_t tcDataLen, uint32_t *state, object_id_t objectId) override;
  ReturnValue_t handleReply(const CommandMessage *reply, Command_t previousCommand, uint32_t *state,
                            CommandMessage *optionalNextCommand, object_id_t objectId,
                            bool *isStep) override;

 private:
  ReturnValue_t checkAndAcquireTargetID(object_id_t *objectIdToSet, const uint8_t *tcData,
                                        uint32_t tcDataLen);
  ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t *MessageQueueToSet,
                                                     object_id_t *objectId);

  ReturnValue_t prepareModeReply(const CommandMessage *reply, object_id_t objectId);
  ReturnValue_t prepareWrongModeReply(const CommandMessage *reply, object_id_t objectId);
  ReturnValue_t prepareCantReachModeReply(const CommandMessage *reply, object_id_t objectId);

  enum Subservice {  //!< [EXPORT] : [COMMENT] Mode Commanding Subservices
    //!< [EXPORT] : [COMMAND] Command assembly, subsystem or device mode
    COMMAND_MODE_COMMAND = 1,
    //!< [EXPORT] : [COMMAND] Command to set the specified Mode,
    //! regardless of external control flag
    COMMAND_MODE_COMMAND_FORCED = 2,
    //!< [EXPORT] : [COMMAND] Read the current mode and
    //! reply with a REPLY_MODE_REPLY
    COMMAND_MODE_READ = 3,
    //!< [EXPORT] : [COMMAND] Trigger an ModeInfo Event.
    //! This command does NOT have a reply
    COMMAND_MODE_ANNCOUNCE = 4,
    //!< [EXPORT] : [COMMAND] Trigger a ModeInfo Event and to send this
    //! command to every child. This command does NOT have a reply.
    COMMAND_MODE_ANNOUNCE_RECURSIVELY = 5,
    //!< [EXPORT] : [REPLY] Reply to a CMD_MODE_COMMAND or CMD_MODE_READ
    REPLY_MODE_REPLY = 6,
    //!< [EXPORT] : [REPLY] Reply in case a mode command can't be executed.
    REPLY_CANT_REACH_MODE = 7,
    //!< [EXPORT] : [REPLY] Reply to a CMD_MODE_COMMAND, indicating that a
    //! mode was commanded and a transition started but was aborted,
    //! the parameters contain the mode that was reached
    REPLY_WRONG_MODE_REPLY = 8
  };

  enum modeParameters { MODE_OFF = 0, MODE_ON = 1, MODE_NORMAL = 2, MODE_RAW = 3 };
};

#endif /* FSFW_PUS_CSERVICE200MODECOMMANDING_H_ */
