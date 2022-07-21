#include "fsfw/tmtcservices/PusServiceBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
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
    //		if(status != MessageQueueIF::EMPTY) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    //			sif::debug << "PusServiceBase::performOperation: Receiving from "
    //					<< "MQ ID: " << std::hex << "0x" << std::setw(8)
    //					<< std::setfill('0') << this->requestQueue->getId()
    //					<< std::dec << " returned: " << status << std::setfill(' ')
    //					<<  std::endl;
#endif
    //		}
    if (status == MessageQueueIF::EMPTY) {
      status = RETURN_OK;
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
    const uint8_t* dataPtr;
    size_t dataLen = 0;
    result = ipcStore->getData(message.getStorageId(), &dataPtr, &dataLen);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Warning?
      continue;
    }

    result = currentPacket.setReadOnlyData(dataPtr, dataLen);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Warning?
      continue;
    }
    result = currentPacket.parseData();
    if (result != HasReturnvaluesIF::RETURN_OK) {
      // TODO: Warning?
      continue;
    }
    result = this->handleRequest(currentPacket.getSubService());
    if (result == RETURN_OK) {
      this->verifyReporter.sendSuccessReport(tc_verification::COMPLETION_SUCCESS,
                                             &this->currentPacket);
    } else {
      this->verifyReporter.sendFailureReport(tc_verification::COMPLETION_FAILURE,
                                             &this->currentPacket, result, 0, errorParameter1,
                                             errorParameter2);
    }
    ipcStore->deleteData(message.getStorageId());
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
  if (ipcStore == nullptr) {
    ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
    if (ipcStore == nullptr) {
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

void PusServiceBase::setCustomIpcStore(StorageManagerIF* ipcStore_) { ipcStore = ipcStore_; }

void PusServiceBase::setCustomErrorReporter(InternalErrorReporterIF* errReporter_) {
  errReporter = errReporter_;
}

void PusServiceBase::initializeTmHelpers(TmSendHelper& tmSendHelper, TmStoreHelper& tmStoreHelper) {
  initializeTmSendHelper(tmSendHelper);
  initializeTmStoreHelper(tmStoreHelper);
}

void PusServiceBase::initializeTmSendHelper(TmSendHelper& tmSendHelper) {
  tmSendHelper.setMsgSource(requestQueue->getId());
  tmSendHelper.setMsgDestination(requestQueue->getDefaultDestination());
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
