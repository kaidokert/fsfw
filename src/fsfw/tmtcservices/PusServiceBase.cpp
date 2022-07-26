#include "fsfw/tmtcservices/PusServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/PUSDistributorIF.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"
#include "fsfw/tmtcservices/TmTcMessage.h"
#include "fsfw/tmtcservices/tcHelpers.h"

object_id_t PusServiceBase::PACKET_DESTINATION = 0;
object_id_t PusServiceBase::PUS_DISTRIBUTOR = 0;

PusServiceBase::PusServiceBase(PsbParams params)
    : SystemObject(params.objectId), psbParams(params) {}

PusServiceBase::~PusServiceBase() {
  if (ownedQueue) {
    QueueFactory::instance()->deleteMessageQueue(psbParams.reqQueue);
  }
}

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
    ReturnValue_t status = psbParams.reqQueue->receiveMessage(&message);
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
          psbParams.serviceId, status);
#endif
      break;
    }
    result = tc::prepareTcReader(*psbParams.tcPool, message.getStorageId(), currentPacket);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      // We were not even able to retrieve the TC, so we can not retrieve any TC properties either
      // without segfaulting
      auto verifParams = VerifFailureParams(tcverif::START_FAILURE, 0, 0, result);
      verifParams.errorParam1 = errorParameter1;
      verifParams.errorParam2 = errorParameter2;
      psbParams.verifReporter->sendFailureReport(verifParams);
      continue;
    }
    result = handleRequest(currentPacket.getSubService());
    if (result == RETURN_OK) {
      psbParams.verifReporter->sendSuccessReport(
          VerifSuccessParams(tcverif::COMPLETION_SUCCESS, currentPacket));
    } else {
      auto failParams = VerifFailureParams(tcverif::COMPLETION_FAILURE, currentPacket, result);
      failParams.errorParam1 = errorParameter1;
      failParams.errorParam2 = errorParameter2;
      psbParams.verifReporter->sendFailureReport(failParams);
    }
    psbParams.tcPool->deleteData(message.getStorageId());
    errorParameter1 = 0;
    errorParameter2 = 0;
  }
}

uint16_t PusServiceBase::getIdentifier() { return psbParams.serviceId; }

MessageQueueId_t PusServiceBase::getRequestQueue() {
  if (psbParams.reqQueue == nullptr) {
    return MessageQueueIF::NO_QUEUE;
  }
  return psbParams.reqQueue->getId();
}

ReturnValue_t PusServiceBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != RETURN_OK) {
    return result;
  }
  if (psbParams.reqQueue == nullptr) {
    ownedQueue = true;
    psbParams.reqQueue = QueueFactory::instance()->createMessageQueue(PSB_DEFAULT_QUEUE_DEPTH);
  } else {
    ownedQueue = false;
  }

  if (psbParams.tmReceiver == nullptr) {
    psbParams.tmReceiver = ObjectManager::instance()->get<AcceptsTelemetryIF>(PACKET_DESTINATION);
    if (psbParams.tmReceiver != nullptr) {
      psbParams.reqQueue->setDefaultDestination(psbParams.tmReceiver->getReportReceptionQueue());
    }
  }

  if (psbParams.pusDistributor == nullptr) {
    psbParams.pusDistributor = ObjectManager::instance()->get<PUSDistributorIF>(PUS_DISTRIBUTOR);
    if (psbParams.pusDistributor != nullptr) {
      registerService(*psbParams.pusDistributor);
    }
  }

  if (psbParams.tcPool == nullptr) {
    psbParams.tcPool = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (psbParams.tcPool == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }

  if (psbParams.verifReporter == nullptr) {
    psbParams.verifReporter =
        ObjectManager::instance()->get<VerificationReporterIF>(objects::TC_VERIFICATOR);
    if (psbParams.verifReporter == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void PusServiceBase::setTcPool(StorageManagerIF& tcPool) { psbParams.tcPool = &tcPool; }

void PusServiceBase::setErrorReporter(InternalErrorReporterIF& errReporter_) {
  psbParams.errReporter = &errReporter_;
}

void PusServiceBase::initializeTmHelpers(TmSendHelper& tmSendHelper, TmStoreHelper& tmStoreHelper) {
  initializeTmSendHelper(tmSendHelper);
  initializeTmStoreHelper(tmStoreHelper);
}

void PusServiceBase::initializeTmSendHelper(TmSendHelper& tmSendHelper) {
  if (psbParams.reqQueue != nullptr) {
    tmSendHelper.setMsgQueue(*psbParams.reqQueue);
    tmSendHelper.setDefaultDestination(psbParams.reqQueue->getDefaultDestination());
  }

  if (psbParams.errReporter == nullptr) {
    psbParams.errReporter =
        ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    if (psbParams.errReporter != nullptr) {
      tmSendHelper.setInternalErrorReporter(*psbParams.errReporter);
    }
  } else {
    tmSendHelper.setInternalErrorReporter(*psbParams.errReporter);
  }
}

void PusServiceBase::initializeTmStoreHelper(TmStoreHelper& tmStoreHelper) const {
  tmStoreHelper.setApid(psbParams.apid);
}

void PusServiceBase::setVerificationReporter(VerificationReporterIF& reporter) {
  psbParams.verifReporter = &reporter;
}

ReturnValue_t PusServiceBase::registerService(PUSDistributorIF& distributor) {
  return distributor.registerService(this);
}

void PusServiceBase::setTmReceiver(AcceptsTelemetryIF& tmReceiver_) {
  psbParams.tmReceiver = &tmReceiver_;
}

void PusServiceBase::setRequestQueue(MessageQueueIF& reqQueue) { psbParams.reqQueue = &reqQueue; }
