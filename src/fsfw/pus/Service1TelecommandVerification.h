#ifndef FSFW_PUS_SERVICE1TELECOMMANDVERIFICATION_H_
#define FSFW_PUS_SERVICE1TELECOMMANDVERIFICATION_H_

#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tmtcservices/AcceptsVerifyMessageIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"
#include "fsfw/tmtcservices/TmSendHelper.h"
#include "fsfw/tmtcservices/TmStoreAndSendHelper.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"

/**
 * @brief Verify TC acceptance, start, progress and execution.
 *
 * Full Documentation: ECSS-E70-41A p.51
 *
 * The telecommand verification service provides the capability for
 * explicit verification of each distinct stage of execution of a telecommand
 * packet, from on-board acceptance through to completion of execution.
 *
 * Minimum capabilities of this service:
 *
 *  - TM[1,1]: Telecommand Acceptance Report - Success.
 *  - TM[1,2]: Telecommand Acceptance Report - Failure.
 *
 * Additional capabilities of this service:
 *
 *  - TM[1,3]: Telecommand Execution Started Report - Success (Req. 4).
 *  - TM[1,4]: Telecommand Execution Started Report - Failure (Req. 3).
 *  - TM[1,5]: Telecommand Execution Progress Report - Success (Req. 6).
 *  - TM[1,6]: Telecommand Execution Progress Report - Failure (Req. 5).
 *  - TM[1,7]: Telecommand Execution Completed Report - Success (Req. 8).
 *  - TM[1,8]: Telecommand Execution Completed Report - Failure (Req. 7).
 *
 * This Service is not inherited from PUSServiceBase unlike other PUS Services
 * because all services implementing PUSServiceBase use this service to
 * generate verification reports.
 * @ingroup pus_services
 */
class Service1TelecommandVerification : public AcceptsVerifyMessageIF,
                                        public SystemObject,
                                        public ExecutableObjectIF {
 public:
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_1;

  Service1TelecommandVerification(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                                  object_id_t targetDestination, uint16_t messageQueueDepth,
                                  TimeWriterIF* timeStamper = nullptr);
  ~Service1TelecommandVerification() override;

  /**
   *
   * @return ID of Verification Queue
   */
  MessageQueueId_t getVerificationQueue() override;

  /**
   * Performs the service periodically as specified in init_mission().
   * Triggers the handlePacket function to send TC verification messages
   * @param operationCode
   * @return
   */
  ReturnValue_t performOperation(uint8_t operationCode) override;

  /**
   * Initializes the destination for TC verification messages and initializes
   * Service 1 as a system object
   * @return
   */
  ReturnValue_t initialize() override;

 private:
  uint16_t apid = 0;
  uint8_t serviceId = 0;

  object_id_t targetDestination = objects::NO_OBJECT;

  ReturnValue_t sendVerificationReport(PusVerificationMessage* message);
  ReturnValue_t generateFailureReport(PusVerificationMessage* message);
  ReturnValue_t generateSuccessReport(PusVerificationMessage* message);

  uint16_t packetSubCounter = 0;

  TmSendHelper sendHelper;
  TmStoreHelper storeHelper;
  TmStoreAndSendWrapper tmHelper;
  InternalErrorReporterIF* errReporter = nullptr;
  TimeWriterIF* timeStamper = nullptr;
  StorageManagerIF* tmStore = nullptr;
  MessageQueueIF* tmQueue = nullptr;

  enum class Subservice : uint8_t {
    VERIFY_ACCEPTANCE_SUCCESS = 1,  //!< [EXPORT] : [TM]
    VERIFY_ACCEPTANCE_FAILED = 2,   //!< [EXPORT] : [TM]
    VERIFY_START_SUCCESS = 3,       //!< [EXPORT] : [TM]
    VERIFY_START_FAILED = 4,        //!< [EXPORT] : [TM]
    VERIFY_STEP_SUCCESS = 5,        //!< [EXPORT] : [TM]
    VERIFY_STEP_FAILED = 6          //!< [EXPORT] : [TM]
  };
};

#endif /* FSFW_PUS_SERVICE1TELECOMMANDVERIFICATION_H_ */
