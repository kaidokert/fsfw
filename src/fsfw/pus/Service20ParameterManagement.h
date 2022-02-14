#ifndef FSFW_PUS_SERVICE20PARAMETERMANAGEMENT_H_
#define FSFW_PUS_SERVICE20PARAMETERMANAGEMENT_H_

#include "fsfw/tmtcservices/CommandingServiceBase.h"

/**
 * @brief   PUS Service 20 Parameter Service implementation
 * @details
 * This service handles PUS service requests related to parameter management and forwards
 * them to the internal software bus.
 * @author  J. Gerhards
 *
 */
class Service20ParameterManagement : public CommandingServiceBase {
 public:
  Service20ParameterManagement(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                               uint8_t numberOfParallelCommands = 4,
                               uint16_t commandTimeoutSeconds = 60);
  virtual ~Service20ParameterManagement();

  static constexpr uint8_t NUM_OF_PARALLEL_COMMANDS = 4;
  static constexpr uint16_t COMMAND_TIMEOUT_SECONDS = 60;

 protected:
  /* CommandingServiceBase (CSB) abstract functions. See CSB documentation. */
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
  ReturnValue_t checkAndAcquireTargetID(object_id_t* objectIdToSet, const uint8_t* tcData,
                                        size_t tcDataLen);
  ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t* messageQueueToSet,
                                                     object_id_t* objectId);
  ReturnValue_t prepareDirectCommand(CommandMessage* message, const uint8_t* tcData,
                                     size_t tcDataLen);

  ReturnValue_t prepareDumpCommand(CommandMessage* message, const uint8_t* tcData,
                                   size_t tcDataLen);
  ReturnValue_t prepareLoadCommand(CommandMessage* message, const uint8_t* tcData,
                                   size_t tcDataLen);

  enum class Subservice {
    PARAMETER_LOAD = 128,        //!< [EXPORT] : Load a Parameter
    PARAMETER_DUMP = 129,        //!< [EXPORT] : Dump a Parameter
    PARAMETER_DUMP_REPLY = 130,  //!< [EXPORT] : Dump a Parameter
  };
};

#endif /* FSFW_PUS_SERVICE20PARAMETERMANAGEMENT_H_ */
