#include "fsfw/tmtcservices/PusServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tcdistribution/PUSDistributorIF.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

object_id_t PusServiceBase::packetSource = 0;
object_id_t PusServiceBase::packetDestination = 0;

PusServiceBase::PusServiceBase(object_id_t setObjectId, uint16_t setApid, uint8_t setServiceId)
    : SystemObject(setObjectId), apid(setApid), serviceId(setServiceId) {
  requestQueue = QueueFactory::instance()->createMessageQueue(PUS_SERVICE_MAX_RECEPTION);
}

PusServiceBase::~PusServiceBase() { QueueFactory::instance()->deleteMessageQueue(requestQueue); }

ReturnValue_t PusServiceBase::performOperation(uint8_t opCode) {
  handleRequestQueue();
  ReturnValue_t result = this->performService();
  if (result != RETURN_OK) {
    FSFW_LOGWT("performOperation: PUS service {} return with error {}\n", serviceId, result);
    return RETURN_FAILED;
  }
  return RETURN_OK;
}

void PusServiceBase::setTaskIF(PeriodicTaskIF* taskHandle) { this->taskHandle = taskHandle; }

void PusServiceBase::handleRequestQueue() {
  TmTcMessage message;
  ReturnValue_t result = RETURN_FAILED;
  for (uint8_t count = 0; count < PUS_SERVICE_MAX_RECEPTION; count++) {
    ReturnValue_t status = this->requestQueue->receiveMessage(&message);
    //		if(status != MessageQueueIF::EMPTY) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    //			sif::debug << "PusServiceBase::performOperation: Receiving from "
    //					<< "MQ ID: " << std::hex << "0x" << std::setw(8)
    //					<< std::setfill('0') << this->requestQueue->getId()
    //					<< std::dec << " returned: " << status << std::setfill(' ')
    //					<<  std::endl;
#endif
    //		}

    if (status == RETURN_OK) {
      this->currentPacket.setStoreAddress(message.getStorageId(), &currentPacket);
      // info << "Service " << (uint16_t) this->serviceId <<
      //      ": new packet!" << std::endl;

      result = this->handleRequest(currentPacket.getSubService());

      // debug << "Service " << (uint16_t)this->serviceId <<
      //    ": handleRequest returned: " << (int)return_code << std::endl;
      if (result == RETURN_OK) {
        this->verifyReporter.sendSuccessReport(tc_verification::COMPLETION_SUCCESS,
                                               &this->currentPacket);
      } else {
        this->verifyReporter.sendFailureReport(tc_verification::COMPLETION_FAILURE,
                                               &this->currentPacket, result, 0, errorParameter1,
                                               errorParameter2);
      }
      this->currentPacket.deletePacket();
      errorParameter1 = 0;
      errorParameter2 = 0;
    } else if (status == MessageQueueIF::EMPTY) {
      status = RETURN_OK;
      // debug << "PusService " << (uint16_t)this->serviceId <<
      //      ": no new packet." << std::endl;
      break;
    } else {
      FSFW_LOGWT("performOperation: Service {}. Error receiving packed, code {}\n", serviceId,
                 status);
    }
  }
}

uint16_t PusServiceBase::getIdentifier() { return this->serviceId; }

MessageQueueId_t PusServiceBase::getRequestQueue() { return this->requestQueue->getId(); }

ReturnValue_t PusServiceBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != RETURN_OK) {
    return result;
  }
  auto* destService = ObjectManager::instance()->get<AcceptsTelemetryIF>(packetDestination);
  auto* distributor = ObjectManager::instance()->get<PUSDistributorIF>(packetSource);
  if (destService == nullptr or distributor == nullptr) {
    FSFW_LOGWT(
        "ctor: Service {} | Make sure static packetSource and packetDestination "
        "are defined correctly\n",
        serviceId);
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  this->requestQueue->setDefaultDestination(destService->getReportReceptionQueue());
  distributor->registerService(this);
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PusServiceBase::initializeAfterTaskCreation() {
  // If task parameters, for example task frequency are required, this
  // function should be overriden and the system object task IF can
  // be used to get those parameters.
  return HasReturnvaluesIF::RETURN_OK;
}
