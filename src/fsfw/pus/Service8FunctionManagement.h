#ifndef FSFW_PUS_SERVICE8FUNCTIONMANAGEMENT_H_
#define FSFW_PUS_SERVICE8FUNCTIONMANAGEMENT_H_

#include "fsfw/action/ActionMessage.h"
#include "fsfw/tmtcservices/CommandingServiceBase.h"

/**
 * @brief Functional commanding.
 * Full Documentation: ECSS-E-ST-70-41C p.64, p. 451
 * Dissertation Baetz p. 115, 116, 165-167
 *
 * This service provides the capability to perform functions of an
 * application process and provides high-level commanding as opposed to the
 * Raw Access provided by Service 2. Examples for these functions can include
 * control and operation of payload or the AOCS subsystem.
 * This service will be the primary means to control the spacecraft as it is
 * considered safer than the Raw Access provided
 * by Service 2 and is generally sufficient for most tasks.
 *
 * This is a gateway service. It relays device commands using the software bus.
 * This service is very closely tied to the Commanding Service Base template
 * class. There is constant interaction between this Service Base und a
 * subclass like this service.
 *
 * Service Capability:
 * 	 - TC[8,128]: Direct Commanding
 *   - TM[8,130]: Direct Commanding Data Reply
 *
 * @ingroup pus_services
 */
class Service8FunctionManagement : public CommandingServiceBase {
 public:
  Service8FunctionManagement(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                             uint8_t numParallelCommands = 4, uint16_t commandTimeoutSeconds = 60);
  ~Service8FunctionManagement() override;

 protected:
  /* CSB abstract functions implementation . See CSB documentation. */
  ReturnValue_t isValidSubservice(uint8_t subservice) override;
  ReturnValue_t getMessageQueueAndObject(uint8_t subservice, const uint8_t* tcData,
                                         size_t tcDataLen, MessageQueueId_t* id,
                                         object_id_t* objectId) override;
  ReturnValue_t prepareCommand(CommandMessage* message, uint8_t subservice, const uint8_t* tcData,
                               size_t tcDataLen, uint32_t* state, object_id_t objectId) override;
  ReturnValue_t handleReply(const CommandMessage* reply, Command_t previousCommand, uint32_t* state,
                            CommandMessage* optionalNextCommand, object_id_t objectId,
                            bool* isStep) override;

 private:
  enum class Subservice {
    //!< [EXPORT] : [COMMAND] Functional commanding
    COMMAND_DIRECT_COMMANDING = 128,
    //!< [EXPORT] : [REPLY] Data reply
    REPLY_DIRECT_COMMANDING_DATA = 130,
  };

  ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t* messageQueueToSet,
                                                     object_id_t* objectId);
  ReturnValue_t prepareDirectCommand(CommandMessage* message, const uint8_t* tcData,
                                     size_t tcDataLen);
  ReturnValue_t handleDataReply(const CommandMessage* reply, object_id_t objectId,
                                ActionId_t actionId);
};

#endif /* FSFW_PUS_SERVICE8FUNCTIONMANAGEMENT_H_ */
