#ifndef FRAMEWORK_TMTCSERVICES_PUSSERVICEBASE_H_
#define FRAMEWORK_TMTCSERVICES_PUSSERVICEBASE_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "../objectmanager/SystemObject.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../tmtcpacket/pus/TcPacketStored.h"
#include "AcceptsTelecommandsIF.h"
#include "VerificationCodes.h"
#include "VerificationReporter.h"
#include "../ipc/MessageQueueIF.h"

namespace Factory{
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
        public SystemObject,
        public HasReturnvaluesIF {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	/**
	 * @brief	The passed values are set, but inter-object initialization is
	 * 			done in the initialize method.
	 * @param setObjectId
	 * The system object identifier of this Service instance.
	 * @param setApid
	 * The APID the Service is instantiated for.
	 * @param setServiceId
	 * The Service Identifier as specified in ECSS PUS.
	 */
	PusServiceBase(	object_id_t setObjectId, uint16_t setApid,
			uint8_t setServiceId);
	/**
	 * The destructor is empty.
	 */
	virtual ~PusServiceBase();
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
	 * 			On success, RETURN_OK shall be returned.
	 */
	virtual ReturnValue_t handleRequest(uint8_t subservice) = 0;
	/**
	 * In performService, implementations can handle periodic,
	 * non-TC-triggered activities.
	 * The performService method is always called.
	 * @return	Currently, everything other that RETURN_OK only triggers
	 * 			diagnostic output.
	 */
	virtual ReturnValue_t performService() = 0;
	/**
	 * This method implements the typical activity of a simple PUS Service.
	 * It checks for new requests, and, if found, calls handleRequest, sends
	 * completion verification messages and deletes
	 * the TC requests afterwards.
	 * performService is always executed afterwards.
	 * @return	@c RETURN_OK if the periodic performService was successful.
	 * 			@c RETURN_FAILED else.
	 */
	ReturnValue_t performOperation(uint8_t opCode) override;
	virtual uint16_t getIdentifier() override;
	MessageQueueId_t getRequestQueue() override;
	virtual ReturnValue_t initialize() override;

	virtual void setTaskIF(PeriodicTaskIF* taskHandle) override;
	virtual ReturnValue_t initializeAfterTaskCreation() override;
protected:
	/**
	 * @brief   Handle to the underlying task
	 * @details
	 * Will be set by setTaskIF(), which is called on task creation.
	 */
	PeriodicTaskIF* taskHandle = nullptr;
	/**
	 * The APID of this instance of the Service.
	 */
	uint16_t apid;
	/**
	 * The Service Identifier.
	 */
	uint8_t serviceId;
	/**
	 * One of two error parameters for additional error information.
	 */
	uint32_t errorParameter1 = 0;
	/**
	 * One of two error parameters for additional error information.
	 */
	uint32_t errorParameter2 = 0;
	/**
	 * This is a complete instance of the telecommand reception queue
	 * of the class. It is initialized on construction of the class.
	 */
	MessageQueueIF* requestQueue = nullptr;
	/**
	 * An instance of the VerificationReporter class, that simplifies
	 * sending any kind of verification message to the TC Verification Service.
	 */
	VerificationReporter verifyReporter;
	/**
	 * The current Telecommand to be processed.
	 * It is deleted after handleRequest was executed.
	 */
	TcPacketStored currentPacket;

	static object_id_t packetSource;

	static object_id_t packetDestination;
private:
	/**
	 * This constant sets the maximum number of packets accepted per call.
	 * Remember that one packet must be completely handled in one
	 * #handleRequest call.
	 */
	static const uint8_t PUS_SERVICE_MAX_RECEPTION = 10;

	void handleRequestQueue();
};

#endif /* PUSSERVICEBASE_H_ */
