#include "fsfw/tmtcservices/PusServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/PUSDistributorIF.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"
#include "fsfw/tmtcservices/TmTcMessage.h"
#include "fsfw/tmtcservices/tcHelpers.h"

object_id_t PusServiceBase::packetSource = 0;
object_id_t PusServiceBase::packetDestination = 0;

PusServiceBase::PusServiceBase(object_id_t setObjectId, uint16_t setApid, uint8_t setServiceId,
                               VerificationReporterIF* verifyReporter)
    : SystemObject(setObjectId),
      apid(setApid),
      serviceId(setServiceId),
      verifyReporter(verifyReporter) {
  requestQueue = QueueFactory::instance()->createMessageQueue(PUS_SERVICE_MAX_RECEPTION);
}

PusServiceBase::~PusServiceBase() { QueueFactory::instance()->deleteMessageQueue(requestQueue); }

ReturnValue_t PusServiceBase::performOperation(uint8_t opCode) {
  handleRequestQueue();
  ReturnValue_t result = performService();
  if (result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PusService " << (uint16_t)this->serviceId << ": performService returned with "
               << (int16_t)result << std::endl;
#endif
    return RETURN_FAILED;
  }
  return RETURN_OK;
}

void PusServiceBase::setTaskIF(PeriodicTaskIF* taskHandle_) { this->taskHandle = taskHandle_; }

void PusServiceBase::handleRequestQueue() {
  TmTcMessage message;
  ReturnValue_t result;
  for (uint8_t count = 0; count < PUS_SERVICE_MAX_RECEPTION; count++) {
    ReturnValue_t status = this->requestQueue->receiveMessage(&message);
    if (status == MessageQueueIF::EMPTY) {
      break;
    } else if (status != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "PusServiceBase::performOperation: Service " << this->serviceId
                 << ": Error receiving packet. Code: " << std::hex << status << std::dec
                 << std::endl;
#else
      sif::printError(
          "PusServiceBase::performOperation: Service %d. Error receiving packet. Code: %04x\n",
          serviceId, status);
#endif
      break;
    }
    result = tc::prepareTcReader(tcStore, message.getStorageId(), currentPacket);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      auto params = VerifFailureParams(tcverif::START_FAILURE, currentPacket, result);
      params.errorParam1 = errorParameter1;
      params.errorParam2 = errorParameter2;
      verifyReporter->sendFailureReport(params);
      continue;
    }
    result = handleRequest(currentPacket.getSubService());
    if (result == RETURN_OK) {
      verifyReporter->sendSuccessReport(
          VerifSuccessParams(tcverif::COMPLETION_SUCCESS, currentPacket));
    } else {
      auto params = VerifFailureParams(tcverif::COMPLETION_FAILURE, currentPacket, result);
      params.errorParam1 = errorParameter1;
      params.errorParam2 = errorParameter2;
      verifyReporter->sendFailureReport(params);
    }
    tcStore->deleteData(message.getStorageId());
    errorParameter1 = 0;
    errorParameter2 = 0;
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PusServiceBase::PusServiceBase: Service " << this->serviceId
               << ": Configuration error. Make sure "
               << "packetSource and packetDestination are defined correctly" << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  this->requestQueue->setDefaultDestination(destService->getReportReceptionQueue());
  distributor->registerService(this);
  if (tcStore == nullptr) {
    tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
    if (tcStore == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  if (verifyReporter == nullptr) {
    verifyReporter =
        ObjectManager::instance()->get<VerificationReporterIF>(objects::TC_VERIFICATOR);
    if (verifyReporter == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PusServiceBase::initializeAfterTaskCreation() {
  // If task parameters, for example task frequency are required, this
  // function should be overriden and the system object task IF can
  // be used to get those parameters.
  return HasReturnvaluesIF::RETURN_OK;
}

void PusServiceBase::setCustomTcStore(StorageManagerIF* tcStore_) { tcStore = tcStore_; }

void PusServiceBase::setCustomErrorReporter(InternalErrorReporterIF* errReporter_) {
  errReporter = errReporter_;
}

void PusServiceBase::initializeTmHelpers(TmSendHelper& tmSendHelper, TmStoreHelper& tmStoreHelper) {
  initializeTmSendHelper(tmSendHelper);
  initializeTmStoreHelper(tmStoreHelper);
}

void PusServiceBase::initializeTmSendHelper(TmSendHelper& tmSendHelper) {
  tmSendHelper.setMsgQueue(*requestQueue);
  tmSendHelper.setDefaultDestination(requestQueue->getDefaultDestination());
  if (errReporter == nullptr) {
    errReporter =
        ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    if (errReporter != nullptr) {
      tmSendHelper.setInternalErrorReporter(errReporter);
    }
  }
}

void PusServiceBase::initializeTmStoreHelper(TmStoreHelper& tmStoreHelper) const {
  tmStoreHelper.setApid(apid);
}

void PusServiceBase::setVerificationReporter(VerificationReporterIF* reporter) {
  verifyReporter = reporter;
}
