#include <fsfw/housekeeping/HousekeepingSnapshot.h>
#include <fsfw/internalerror/InternalErrorReporter.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/timemanager/CCSDSTime.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"
#include "fsfw/action/ActionMessage.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "mocks/PeriodicTaskIFMock.h"

TEST_CASE("Internal Error Reporter", "[TestInternalError]") {
  PeriodicTaskMock task(10, nullptr);
  ObjectManagerIF* manager = ObjectManager::instance();
  if (manager == nullptr) {
    FAIL();
  }
  auto* internalErrorReporter = dynamic_cast<InternalErrorReporter*>(
      ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER));
  if (internalErrorReporter == nullptr) {
    FAIL();
  }
  task.addComponent(objects::INTERNAL_ERROR_REPORTER);
  // This calls the initializeAfterTaskCreation function
  task.startTask();
  MessageQueueIF* testQueue = QueueFactory::instance()->createMessageQueue(1);
  MessageQueueIF* hkQueue = QueueFactory::instance()->createMessageQueue(1);
  internalErrorReporter->getSubscriptionInterface()->subscribeForSetUpdateMessage(
      InternalErrorDataset::ERROR_SET_ID, objects::NO_OBJECT, hkQueue->getId(), true);
  auto* ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  SECTION("MessageQueueFull") {
    CommandMessage message;
    ActionMessage::setCompletionReply(&message, 10, true);
    auto result = hkQueue->sendMessage(testQueue->getId(), &message);
    REQUIRE(result == returnvalue::OK);
    uint32_t queueHits = 0;
    uint32_t lostTm = 0;
    uint32_t storeHits = 0;
    /* We don't know if another test caused a queue Hit so we will enforce one,
     then remeber the queueHit count and force another hit */
    internalErrorReporter->queueMessageNotSent();
    internalErrorReporter->performOperation(0);
    {
      CommandMessage hkMessage;
      result = hkQueue->receiveMessage(&hkMessage);
      REQUIRE(result == returnvalue::OK);
      REQUIRE(hkMessage.getCommand() == HousekeepingMessage::UPDATE_SNAPSHOT_SET);
      store_address_t storeAddress;
      gp_id_t gpid =
          HousekeepingMessage::getUpdateSnapshotVariableCommand(&hkMessage, &storeAddress);
      REQUIRE(gpid.objectId == objects::INTERNAL_ERROR_REPORTER);
      // We need the object ID of the reporter here (NO_OBJECT)
      InternalErrorDataset dataset(objects::INTERNAL_ERROR_REPORTER);
      CCSDSTime::CDS_short time{};
      ConstAccessorPair data = ipcStore->getData(storeAddress);
      REQUIRE(data.first == returnvalue::OK);
      HousekeepingSnapshot hkSnapshot(&time, &dataset);
      const uint8_t* buffer = data.second.data();
      size_t size = data.second.size();
      result = hkSnapshot.deSerialize(&buffer, &size, SerializeIF::Endianness::MACHINE);
      REQUIRE(result == returnvalue::OK);
      // Remember the amount of queueHits before to see the increase
      queueHits = dataset.queueHits.value;
      lostTm = dataset.tmHits.value;
      storeHits = dataset.storeHits.value;
    }
    result = hkQueue->sendMessage(testQueue->getId(), &message);
    REQUIRE(result == MessageQueueIF::FULL);
    internalErrorReporter->lostTm();
    internalErrorReporter->storeFull();
    {
      internalErrorReporter->performOperation(0);
      CommandMessage hkMessage;
      result = hkQueue->receiveMessage(&hkMessage);
      REQUIRE(result == returnvalue::OK);
      REQUIRE(hkMessage.getCommand() == HousekeepingMessage::UPDATE_SNAPSHOT_SET);
      store_address_t storeAddress;
      gp_id_t gpid =
          HousekeepingMessage::getUpdateSnapshotVariableCommand(&hkMessage, &storeAddress);
      REQUIRE(gpid.objectId == objects::INTERNAL_ERROR_REPORTER);

      ConstAccessorPair data = ipcStore->getData(storeAddress);
      REQUIRE(data.first == returnvalue::OK);
      CCSDSTime::CDS_short time;
      // We need the object ID of the reporter here (NO_OBJECT)
      InternalErrorDataset dataset(objects::INTERNAL_ERROR_REPORTER);
      HousekeepingSnapshot hkSnapshot(&time, &dataset);
      const uint8_t* buffer = data.second.data();
      size_t size = data.second.size();
      result = hkSnapshot.deSerialize(&buffer, &size, SerializeIF::Endianness::MACHINE);
      REQUIRE(result == returnvalue::OK);
      // Test that we had one more queueHit
      REQUIRE(dataset.queueHits.value == (queueHits + 1));
      REQUIRE(dataset.tmHits.value == (lostTm + 1));
      REQUIRE(dataset.storeHits.value == (storeHits + 1));
    }
    // Complete Coverage
    internalErrorReporter->setDiagnosticPrintout(true);
    internalErrorReporter->setMutexTimeout(MutexIF::TimeoutType::BLOCKING, 0);
    {
      // Message Queue Id
      MessageQueueId_t id = internalErrorReporter->getCommandQueue();
      REQUIRE(id != MessageQueueIF::NO_QUEUE);
      CommandMessage message2;
      sid_t sid(objects::INTERNAL_ERROR_REPORTER, InternalErrorDataset::ERROR_SET_ID);
      HousekeepingMessage::setToggleReportingCommand(&message2, sid, true, false);
      result = hkQueue->sendMessage(id, &message2);
      REQUIRE(result == returnvalue::OK);
      internalErrorReporter->performOperation(0);
    }
  }
  QueueFactory::instance()->deleteMessageQueue(testQueue);
  QueueFactory::instance()->deleteMessageQueue(hkQueue);
}
