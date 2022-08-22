#include "fsfw/monitoring/LimitViolationReporter.h"

#include "fsfw/monitoring/MonitoringIF.h"
#include "fsfw/monitoring/ReceivesMonitoringReportsIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/SerializeAdapter.h"

ReturnValue_t LimitViolationReporter::sendLimitViolationReport(const SerializeIF* data) {
  ReturnValue_t result = checkClassLoaded();
  if (result != returnvalue::OK) {
    return result;
  }
  store_address_t storeId;
  uint8_t* dataTarget = nullptr;
  size_t maxSize = data->getSerializedSize();
  if (maxSize > MonitoringIF::VIOLATION_REPORT_MAX_SIZE) {
    return MonitoringIF::INVALID_SIZE;
  }
  result = ipcStore->getFreeElement(&storeId, maxSize, &dataTarget);
  if (result != returnvalue::OK) {
    return result;
  }
  size_t size = 0;
  result = data->serialize(&dataTarget, &size, maxSize, SerializeIF::Endianness::BIG);
  if (result != returnvalue::OK) {
    return result;
  }
  CommandMessage report;
  MonitoringMessage::setLimitViolationReport(&report, storeId);
  return MessageQueueSenderIF::sendMessage(reportQueue, &report);
}

ReturnValue_t LimitViolationReporter::checkClassLoaded() {
  if (reportQueue == 0) {
    ReceivesMonitoringReportsIF* receiver =
        ObjectManager::instance()->get<ReceivesMonitoringReportsIF>(reportingTarget);
    if (receiver == nullptr) {
      return ObjectManagerIF::NOT_FOUND;
    }
    reportQueue = receiver->getCommandQueue();
  }
  if (ipcStore == nullptr) {
    ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
    if (ipcStore == nullptr) {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

// Lazy initialization.
MessageQueueId_t LimitViolationReporter::reportQueue = 0;
StorageManagerIF* LimitViolationReporter::ipcStore = nullptr;
object_id_t LimitViolationReporter::reportingTarget = 0;
