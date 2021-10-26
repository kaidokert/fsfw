#include "TestController.h"
#include "OBSWConfig.h"

#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

TestController::TestController(object_id_t objectId, object_id_t device0, object_id_t device1,
        size_t commandQueueDepth):
        ExtendedControllerBase(objectId, objects::NO_OBJECT, commandQueueDepth),
        deviceDataset0(device0),
        deviceDataset1(device1) {
}

TestController::~TestController() {
}

ReturnValue_t TestController::handleCommandMessage(CommandMessage *message) {
    return HasReturnvaluesIF::RETURN_OK;
}

void TestController::performControlOperation() {
    /* We will trace vaiables if we received an update notification or snapshots */
#if OBSW_CONTROLLER_PRINTOUT == 1
    if(not traceVariable) {
        return;
    }

    switch(currentTraceType) {
    case(NONE): {
        break;
    }
    case(TRACE_DEV_0_UINT8): {
        if(traceCounter == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Tracing finished" << std::endl;
#else
            sif::printInfo("Tracing finished\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
            traceVariable = false;
            traceCounter = traceCycles;
            currentTraceType = TraceTypes::NONE;
            break;
        }

        PoolReadGuard readHelper(&deviceDataset0.testUint8Var);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Tracing device 0 variable 0 (UINT8), current value: " <<
                static_cast<int>(deviceDataset0.testUint8Var.value) << std::endl;
#else
        sif::printInfo("Tracing device 0 variable 0 (UINT8), current value: %d\n",
                deviceDataset0.testUint8Var.value);
#endif
        traceCounter--;
        break;
    }
    case(TRACE_DEV_0_VECTOR): {
        break;
    }

    }
#endif /* OBSW_CONTROLLER_PRINTOUT == 1 */
}

void TestController::handleChangedDataset(sid_t sid, store_address_t storeId, bool* clearMessage) {
    using namespace std;

#if OBSW_CONTROLLER_PRINTOUT == 1
    char const* printout = nullptr;
    if(storeId == storeId::INVALID_STORE_ADDRESS) {
        printout = "Notification";
    }
    else {
        printout = "Snapshot";
    }
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "TestController::handleChangedDataset: " << printout << " update from object "
            "ID " << setw(8) << setfill('0') << hex << sid.objectId <<
            " and set ID " << sid.ownerSetId << dec << setfill(' ') << endl;
#else
    sif::printInfo("TestController::handleChangedPoolVariable: %s update from object ID 0x%08x and "
            "set ID %lu\n", printout, sid.objectId, sid.ownerSetId);
#endif

    if (storeId == storeId::INVALID_STORE_ADDRESS) {
        if(sid.objectId == objects::TEST_DEVICE_HANDLER_0) {
            PoolReadGuard readHelper(&deviceDataset0.testFloat3Vec);
            float floatVec[3];
            floatVec[0] = deviceDataset0.testFloat3Vec.value[0];
            floatVec[1] = deviceDataset0.testFloat3Vec.value[1];
            floatVec[2] = deviceDataset0.testFloat3Vec.value[2];
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Current float vector (3) values: ["  << floatVec[0] << ", " <<
                    floatVec[1] << ", " << floatVec[2] << "]" << std::endl;
#else
            sif::printInfo("Current float vector (3) values: [%f, %f, %f]\n",
                    floatVec[0], floatVec[1], floatVec[2]);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        }
    }
#endif /* OBSW_CONTROLLER_PRINTOUT == 1 */

    /* We will trace the variables for snapshots and update notifications */
    if(not traceVariable) {
        traceVariable = true;
        traceCounter = traceCycles;
        currentTraceType = TraceTypes::TRACE_DEV_0_VECTOR;
    }
}

void TestController::handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
        bool* clearMessage) {
    using namespace std;

#if OBSW_CONTROLLER_PRINTOUT == 1
    char const* printout = nullptr;
    if (storeId == storeId::INVALID_STORE_ADDRESS) {
        printout = "Notification";
    }
    else {
        printout = "Snapshot";
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "TestController::handleChangedPoolVariable: " << printout << " update from object "
            "ID 0x" << setw(8) << setfill('0') << hex << globPoolId.objectId <<
            " and local pool ID " << globPoolId.localPoolId << dec << setfill(' ') << endl;
#else
    sif::printInfo("TestController::handleChangedPoolVariable: %s update from object ID 0x%08x and "
            "local pool ID %lu\n", printout, globPoolId.objectId, globPoolId.localPoolId);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

    if (storeId == storeId::INVALID_STORE_ADDRESS) {
        if(globPoolId.objectId == objects::TEST_DEVICE_HANDLER_0) {
            PoolReadGuard readHelper(&deviceDataset0.testUint8Var);
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Current test variable 0 (UINT8) value: " << static_cast<int>(
                    deviceDataset0.testUint8Var.value) << std::endl;
#else
            sif::printInfo("Current test variable 0 (UINT8) value %d\n",
                    deviceDataset0.testUint8Var.value);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        }
    }
#endif /* OBSW_CONTROLLER_PRINTOUT == 1 */

    /* We will trace the variables for snapshots and update notifications */
    if(not traceVariable) {
        traceVariable = true;
        traceCounter = traceCycles;
        currentTraceType = TraceTypes::TRACE_DEV_0_UINT8;
    }
}

LocalPoolDataSetBase* TestController::getDataSetHandle(sid_t sid) {
    return nullptr;
}

ReturnValue_t TestController::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
        LocalDataPoolManager &poolManager) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TestController::initializeAfterTaskCreation() {
    namespace td = testdevice;
    HasLocalDataPoolIF* device0 = ObjectManager::instance()->get<HasLocalDataPoolIF>(
            deviceDataset0.getCreatorObjectId());
    if(device0 == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TestController::initializeAfterTaskCreation: Test device handler 0 "
                "handle invalid!" << std::endl;
#else
        sif::printWarning("TestController::initializeAfterTaskCreation: Test device handler 0 "
                "handle invalid!");
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    ProvidesDataPoolSubscriptionIF* subscriptionIF = device0->getSubscriptionInterface();
    if(subscriptionIF != nullptr) {
        /* For DEVICE_0, we only subscribe for notifications */
        subscriptionIF->subscribeForSetUpdateMessage(td::TEST_SET_ID, getObjectId(),
                getCommandQueue(), false);
        subscriptionIF->subscribeForVariableUpdateMessage(td::PoolIds::TEST_UINT8_ID,
                getObjectId(), getCommandQueue(), false);
    }


    HasLocalDataPoolIF* device1 = ObjectManager::instance()->get<HasLocalDataPoolIF>(
            deviceDataset0.getCreatorObjectId());
    if(device1 == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TestController::initializeAfterTaskCreation: Test device handler 1 "
                "handle invalid!" << std::endl;
#else
        sif::printWarning("TestController::initializeAfterTaskCreation: Test device handler 1 "
                "handle invalid!");
#endif
    }

    subscriptionIF = device1->getSubscriptionInterface();
    if(subscriptionIF != nullptr) {
        /* For DEVICE_1, we will subscribe for snapshots */
        subscriptionIF->subscribeForSetUpdateMessage(td::TEST_SET_ID, getObjectId(),
                getCommandQueue(), true);
        subscriptionIF->subscribeForVariableUpdateMessage(td::PoolIds::TEST_UINT8_ID,
                getObjectId(), getCommandQueue(), true);
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TestController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

