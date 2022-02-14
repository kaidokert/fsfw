#ifndef FSFW_PUS_SERVICE2DEVICEACCESS_H_
#define FSFW_PUS_SERVICE2DEVICEACCESS_H_

#include "fsfw/devicehandlers/AcceptsDeviceResponsesIF.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/tmtcservices/CommandingServiceBase.h"

/**
 * @brief   Raw Commanding and Wiretapping of devices.
 * @details
 * Full Documentation: ECSS-E-ST-70-41C or ECSS-E-70-41A
 * Dissertation Baetz p. 115, 116, 165-167.
 *
 * This service provides the capability to communicate with devices in their
 * native protocols with raw commands through the DeviceHandlerIF.
 *
 * This is a gateway service. It relays device commands to the software bus.
 * This service is very closely tied to the CommandingServiceBase
 * template class.
 *
 * There are 4 adaption points for component implementation through the
 * CommandingServiceBase.
 *
 * This service employs custom subservices exclusively.  This includes a
 * wiretapping subservice to monitor all traffic between target devices and
 * this service.
 *
 *   - TC[2,128]: Raw Commanding
 *   - TC[2,129]: Toggle Wiretapping
 *   - TM[2,130]: Wiretapping Packet TM
 *   - TM[2,131]: Wiretapping Packet TC
 * @ingroup pus_services
 */
class Service2DeviceAccess : public CommandingServiceBase, public AcceptsDeviceResponsesIF {
 public:
  Service2DeviceAccess(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                       uint8_t numberOfParallelCommands = 4, uint16_t commandTimeoutSeconds = 60);
  virtual ~Service2DeviceAccess();

 protected:
  //! CommandingServiceBase (CSB) abstract functions. See CSB documentation.
  ReturnValue_t isValidSubservice(uint8_t subservice) override;
  ReturnValue_t getMessageQueueAndObject(uint8_t subservice, const uint8_t* tcData,
                                         size_t tcDataLen, MessageQueueId_t* id,
                                         object_id_t* objectId) override;
  ReturnValue_t prepareCommand(CommandMessage* message, uint8_t subservice, const uint8_t* tcData,
                               size_t tcDataLen, uint32_t* state, object_id_t objectId) override;
  ReturnValue_t handleReply(const CommandMessage* reply, Command_t previousCommand, uint32_t* state,
                            CommandMessage* optionalNextCommand, object_id_t objectId,
                            bool* isStep) override;

  /**
   * @brief 	Generates TM packets containing either the TC wiretapping
   * 			packets or the TM wiretapping packets.
   * Note that for service 2, all telemetry will be treated as an
   * unrequested reply regardless of wiretapping mode.
   * @param reply
   */
  void handleUnrequestedReply(CommandMessage* reply) override;

  MessageQueueId_t getDeviceQueue() override;

 private:
  /**
   * Generates TM packets for Wiretapping Service
   * @param reply
   * @param subservice
   */
  void sendWiretappingTm(CommandMessage* reply, uint8_t subservice);

  ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t* messageQueueToSet,
                                                     object_id_t* objectId);

  ReturnValue_t prepareRawCommand(CommandMessage* messageToSet, const uint8_t* tcData,
                                  size_t tcDataLen);
  ReturnValue_t prepareWiretappingCommand(CommandMessage* messageToSet, const uint8_t* tcData,
                                          size_t tcDataLen);

  enum class Subservice {
    //!< [EXPORT] : [COMMAND] Command in device native protocol
    COMMAND_RAW_COMMANDING = 128,
    //!< [EXPORT] : [COMMAND] Toggle wiretapping of raw communication
    COMMAND_TOGGLE_WIRETAPPING = 129,
    //!< [EXPORT] : [REPLY] Includes wiretapping TM and normal TM raw replies from device
    REPLY_RAW = 130,
    //!< [EXPORT] : [REPLY] Wiretapping packets of commands built by device handler
    REPLY_WIRETAPPING_RAW_TC = 131
  };
};

#endif /* FSFW_PUS_DEVICE2DEVICECOMMANDING_H_ */
