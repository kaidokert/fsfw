#ifndef FSFW_PUS_CSERVICE201HEALTHCOMMANDING_H_
#define FSFW_PUS_CSERVICE201HEALTHCOMMANDING_H_

#include "fsfw/tmtcservices/CommandingServiceBase.h"

/**
 * @brief   Custom PUS service to set health of all objects
 *          implementing hasHealthIF.
 *
 * Examples: Device Handlers, Assemblies or Subsystems.
 * Full Documentation: ECSS-E-ST-70-41C or ECSS-E-70-41A
 * Dissertation Baetz p. 115, 116, 165-167.
 *
 * This is a gateway service. It relays device commands using the software bus.
 * This service is very closely tied to the Commanding Service Base template
 * class. There is constant interaction between this Service Base und a
 * child class like this service
 *
 */
class CService201HealthCommanding : public CommandingServiceBase {
 public:
  CService201HealthCommanding(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                              uint8_t numParallelCommands = 4, uint16_t commandTimeoutSeconds = 60);
  ~CService201HealthCommanding() override = default;

 protected:
  /* CSB abstract function implementations */
  ReturnValue_t isValidSubservice(uint8_t subservice) override;
  ReturnValue_t getMessageQueueAndObject(uint8_t subservice, const uint8_t *tcData,
                                         size_t tcDataLen, MessageQueueId_t *id,
                                         object_id_t *objectId) override;
  /** Prepare health command */
  ReturnValue_t prepareCommand(CommandMessage *message, uint8_t subservice, const uint8_t *tcData,
                               size_t tcDataLen, uint32_t *state, object_id_t objectId) override;
  /** Handle health reply */
  ReturnValue_t handleReply(const CommandMessage *reply, Command_t previousCommand, uint32_t *state,
                            CommandMessage *optionalNextCommand, object_id_t objectId,
                            bool *isStep) override;

 private:
  static ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t *MessageQueueToSet,
                                                            const object_id_t *objectId);

  [[maybe_unused]] ReturnValue_t prepareHealthSetReply(const CommandMessage *reply);

  enum Subservice {
    //! [EXPORT] : [TC] Set health of target object
    COMMAND_SET_HEALTH = 1,
    //! [EXPORT] : [TM] Reply to health set command which also provides old health
    REPLY_HEALTH_SET = 2,
    //! [EXPORT] : [TC] Commands object to announce their health as an event
    COMMAND_ANNOUNCE_HEALTH = 3,
    //! [EXPORT] : [TC] Commands all objects in the health map to announce their health
    COMMAND_ANNOUNCE_HEALTH_ALL = 4
  };
};

#endif /* FSFW_PUS_CSERVICE201HEALTHCOMMANDING_H_ */
