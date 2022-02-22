#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/internalerror/InternalErrorReporter.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/timemanager/CCSDSTime.h>
#include <fsfw/housekeeping/HousekeepingSnapshot.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/action/ActionMessage.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw_tests/unit/CatchDefinitions.h"
#include "fsfw_tests/unit/mocks/PeriodicTaskIFMock.h"

TEST_CASE("Internal Error Reporter", "[TestInternalError]") {
    PeriodicTaskMock task(10);
    ObjectManagerIF* manager = ObjectManager::instance();
    if(manager == nullptr){
        FAIL();
    }
    InternalErrorReporter* internalErrorReporter =
          dynamic_cast<InternalErrorReporter*>(ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER));
    if(internalErrorReporter == nullptr){
        FAIL();
    }
    task.addComponent(objects::INTERNAL_ERROR_REPORTER);
    MessageQueueIF* testQueue = QueueFactory::instance()->createMessageQueue(1);
    MessageQueueIF* hkQueue = QueueFactory::instance()->createMessageQueue(1);
    internalErrorReporter->getSubscriptionInterface()->
      subscribeForSetUpdateMessage(InternalErrorDataset::ERROR_SET_ID, objects::NO_OBJECT, hkQueue->getId(), true);
    StorageManagerIF* ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
    SECTION("MessageQueueFull"){
        CommandMessage message;
        ActionMessage::setCompletionReply(&message, 10, true);
        auto result = hkQueue->sendMessage(testQueue->getId(), &message);
        REQUIRE(result == retval::CATCH_OK);
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
            REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
            REQUIRE(hkMessage.getCommand() == HousekeepingMessage::UPDATE_SNAPSHOT_SET);
            store_address_t storeAddress;
            gp_id_t gpid = HousekeepingMessage::getUpdateSnapshotVariableCommand(&hkMessage, &storeAddress);
            REQUIRE(gpid.objectId == objects::INTERNAL_ERROR_REPORTER);
            // We need the object ID of the reporter here (NO_OBJECT)
            InternalErrorDataset dataset(objects::INTERNAL_ERROR_REPORTER);
            CCSDSTime::CDS_short time;
            ConstAccessorPair data = ipcStore->getData(storeAddress);
            REQUIRE(data.first == HasReturnvaluesIF::RETURN_OK);
            HousekeepingSnapshot hkSnapshot(&time, &dataset);
            const uint8_t* buffer = data.second.data();
            size_t size = data.second.size();
            result = hkSnapshot.deSerialize(&buffer, &size, SerializeIF::Endianness::MACHINE);
            REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
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
            REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
            REQUIRE(hkMessage.getCommand() == HousekeepingMessage::UPDATE_SNAPSHOT_SET);
            store_address_t storeAddress;
            gp_id_t gpid = HousekeepingMessage::getUpdateSnapshotVariableCommand(&hkMessage, &storeAddress);
            REQUIRE(gpid.objectId == objects::INTERNAL_ERROR_REPORTER);
            
            ConstAccessorPair data = ipcStore->getData(storeAddress);
            REQUIRE(data.first == HasReturnvaluesIF::RETURN_OK);
            CCSDSTime::CDS_short time;
            // We need the object ID of the reporter here (NO_OBJECT)
            InternalErrorDataset dataset(objects::INTERNAL_ERROR_REPORTER);
            HousekeepingSnapshot hkSnapshot(&time, &dataset);
            const uint8_t* buffer = data.second.data();
            size_t size = data.second.size();
            result = hkSnapshot.deSerialize(&buffer, &size, SerializeIF::Endianness::MACHINE);
            REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
            // Test that we had one more queueHit 
            REQUIRE(dataset.queueHits.value == (queueHits + 1));
            REQUIRE(dataset.tmHits.value == (lostTm + 1));
            REQUIRE(dataset.storeHits.value == (storeHits + 1));
        }
    }
    QueueFactory::instance()->deleteMessageQueue(testQueue);
    QueueFactory::instance()->deleteMessageQueue(hkQueue);
}