#ifndef FSFW_TMTCSERVICES_PUSSERVICEBASE_H_
#define FSFW_TMTCSERVICES_PUSSERVICEBASE_H_

#include "AcceptsTelecommandsIF.h"
#include "AcceptsTelemetryIF.h"
#include "TmSendHelper.h"
#include "TmStoreHelper.h"
#include "VerificationCodes.h"
#include "VerificationReporter.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tcdistribution/PusDistributorIF.h"

class StorageManagerIF;

/**
 * Configuration parameters for the PUS Service Base
 */
struct PsbParams {
  PsbParams() = default;
  PsbParams(uint16_t apid, AcceptsTelemetryIF* tmReceiver) : apid(apid), tmReceiver(tmReceiver) {}
  PsbParams(const char* name, uint16_t apid, AcceptsTelemetryIF* tmReceiver)
      : name(name), apid(apid), tmReceiver(tmReceiver) {}
  PsbParams(object_id_t objectId, uint16_t apid, uint8_t serviceId)
      : objectId(objectId), apid(apid), serviceId(serviceId) {}
  PsbParams(const char* name, object_id_t objectId, uint16_t apid, uint8_t serviceId)
      : name(name), objectId(objectId), apid(apid), serviceId(serviceId) {}
  const char* name = "";
  object_id_t objectId = objects::NO_OBJECT;
  uint16_t apid = 0;
  uint8_t serviceId = 0;
  /**
   * The default destination ID for generated telemetry. If this is not set, @initialize of PSB
   * will attempt to find a suitable object with the object ID @PusServiceBase::packetDestination
   */
  AcceptsTelemetryIF* tmReceiver = nullptr;
  /**
   * An instance of the VerificationReporter class, that simplifies
   * sending any kind of verification message to the TC Verification Service. If this is not set,
   * @initialize of PSB will attempt to find a suitable global object with the ID
   * @objects::TC_VERIFICATOR
   */
  VerificationReporterIF* verifReporter = nullptr;
  /**
   * This is a complete instance of the telecommand reception queue
   * of the class. It is initialized on construction of the class.
   */
  MessageQueueIF* reqQueue = nullptr;
  /**
   * The internal error reporter which will be used if there  are issues sending telemetry.
   * If this is not set, and the TM send or store helpers are initialized with the PSB,
   * the class will attempt to find a suitable global object with the ID
   * @objects::INTERNAL_ERROR_REPORTER
   */
  InternalErrorReporterIF* errReporter = nullptr;
  /**
   * The storage manager which will be used to retrieve any TC packet using the store ID
   * received via a message. If this is not set, the class will attempt to find a suitable global
   * object with the ID @objects::TC_STORE
   */
  StorageManagerIF* tcPool = nullptr;
  /**
   * Usually, packets are sent via a dedicated PUS distributor. If this distributor is set,
   * the PUS service will register itself there. Otherwise, the base class will try to find
   * a suitable global distributor with the static ID @PusServiceBase::pusDistributor and
   * register itself at that object.
   */
  PusDistributorIF* pusDistributor = nullptr;
  TimeWriterIF* timeStamper = nullptr;
};

namespace Factory {
void setStaticFrameworkObjectIds();
}

/**
 * @defgroup pus_services PUS Service Framework
 * These group contains all implementations of PUS Services in the OBSW.
 * Most of the Services are directly taken from the ECSS PUS Standard.
 */

/**
 * @brief 	This class is the basis for all PUS Services,
 * 			which can immediately process Telecommand Packets.
 * @details
 * It manages Telecommand reception and the generation of Verification Reports.
 * Every class that inherits from this abstract class has to implement
 * handleRequest and performService. Services that are created with this
 * Base class have to handle any kind of request immediately on reception.
 * All PUS Services are System Objects, so an Object ID needs to be specified
 * on construction.
 * @ingroup pus_services
 */
class PusServiceBase : public ExecutableObjectIF,
                       public AcceptsTelecommandsIF,
                       public SystemObject {
  friend void Factory::setStaticFrameworkObjectIds();

 public:
  /**
   * This constant sets the maximum number of packets accepted per call.
   * Remember that one packet must be completely handled in one
   * #handleRequest call.
   */
  static constexpr uint8_t PUS_SERVICE_MAX_RECEPTION = 10;
  static constexpr uint8_t PSB_DEFAULT_QUEUE_DEPTH = 10;

  /**
   * @brief	The passed values are set, but inter-object initialization is
   * 			done in the initialize method.
   * @param params All configuration parameters for the PUS Service Base
   */
  explicit PusServiceBase(PsbParams params);
  /**
   * The destructor is empty.
   */
  ~PusServiceBase() override;

  ReturnValue_t registerService(PusDistributorIF& distributor);
  /**
   * Set the request queue which is used to receive requests. If none is set, the initialize
   * function will create one
   * @param reqQueue
   */
  void setRequestQueue(MessageQueueIF& reqQueue);
  void setTmReceiver(AcceptsTelemetryIF& tmReceiver);
  void setTcPool(StorageManagerIF& tcStore);
  void setVerificationReporter(VerificationReporterIF& reporter);
  void setErrorReporter(InternalErrorReporterIF& errReporter);

  /**
   * Helper methods if the implementing class wants to send telemetry
   * @param tmSendHelper
   */
  ReturnValue_t initializeTmSendHelper(TmSendHelper& tmSendHelper);
  /**
   * Helper methods if the implementing class wants to store telemetry. It will set the correct APID
   * and it will also attempt to set a valid time stamper. If the manually specified time stamper is
   * null, it will attempt to find a suitable one using @objects::TIME_STAMPER
   * @param tmSendHelper
   */
  ReturnValue_t initializeTmStoreHelper(TmStoreHelper& tmStoreHelper) const;
  /**
   * Helper methods if the implementing class wants to both send and store telemetry
   * @param tmSendHelper
   */
  ReturnValue_t initializeTmHelpers(TmSendHelper& tmSendHelper, TmStoreHelper& tmStoreHelper);

  /**
   * @brief 	The handleRequest method shall handle any kind of Telecommand
   * 			Request immediately.
   * @details
   * Implemetations can take the Telecommand in currentPacket and perform
   * any kind of operation.
   * They may send additional "Start Success (1,3)" messages with the
   * verifyReporter, but Completion Success or Failure Reports are generated
   * automatically after execution of this method.
   *
   * If a Telecommand can not be executed within one call cycle,
   * this Base class is not the right parent.
   *
   * The child class may add additional error information by setting
   * #errorParameters which aren attached to the generated verification
   * message.
   *
   * Subservice checking should be implemented in this method.
   *
   * @return	The returned status_code is directly taken as main error code
   * 			in the Verification Report.
   * 			On success, returnvalue::OK shall be returned.
   */
  virtual ReturnValue_t handleRequest(uint8_t subservice) = 0;
  /**
   * In performService, implementations can handle periodic,
   * non-TC-triggered activities.
   * The performService method is always called.
   * @return	Currently, everything other that returnvalue::OK only triggers
   * 			diagnostic output.
   */
  virtual ReturnValue_t performService() = 0;
  /**
   * This method implements the typical activity of a simple PUS Service.
   * It checks for new requests, and, if found, calls handleRequest, sends
   * completion verification messages and deletes
   * the TC requests afterwards.
   * performService is always executed afterwards.
   * @return	@c returnvalue::OK if the periodic performService was successful.
   * 		@c returnvalue::FAILED else.
   */
  ReturnValue_t performOperation(uint8_t opCode) override;
  uint32_t getIdentifier() const override;
  MessageQueueId_t getRequestQueue() const override;
  ReturnValue_t initialize() override;

  void setTaskIF(PeriodicTaskIF* taskHandle) override;
  [[nodiscard]] const char* getName() const override;

 protected:
  /**
   * @brief   Handle to the underlying task
   * @details
   * Will be set by setTaskIF(), which is called on task creation.
   */
  PeriodicTaskIF* taskHandle = nullptr;

  /**
   * One of two error parameters for additional error information.
   */
  uint32_t errorParameter1 = 0;
  /**
   * One of two error parameters for additional error information.
   */
  uint32_t errorParameter2 = 0;
  PsbParams psbParams;
  /**
   * The current Telecommand to be processed.
   * It is deleted after handleRequest was executed.
   */
  PusTcReader currentPacket;
  bool ownedQueue = true;

  static object_id_t PACKET_DESTINATION;
  static object_id_t PUS_DISTRIBUTOR;

 private:
  void handleRequestQueue();
};

#endif /* FSFW_TMTCSERVICES_PUSSERVICEBASE_H_ */
