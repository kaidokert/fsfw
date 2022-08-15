#include "TestController.h"

#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

TestController::TestController(object_id_t objectId, object_id_t parentId, size_t commandQueueDepth)
    : ExtendedControllerBase(objectId, parentId, commandQueueDepth) {}

TestController::~TestController() {}

ReturnValue_t TestController::handleCommandMessage(CommandMessage *message) {
  return returnvalue::OK;
}

void TestController::performControlOperation() {}

void TestController::handleChangedDataset(sid_t sid, store_address_t storeId, bool *clearMessage) {}

void TestController::handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
                                               bool *clearMessage) {}

LocalPoolDataSetBase *TestController::getDataSetHandle(sid_t sid) { return nullptr; }

ReturnValue_t TestController::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                      LocalDataPoolManager &poolManager) {
  return returnvalue::OK;
}

ReturnValue_t TestController::initializeAfterTaskCreation() {
  return ExtendedControllerBase::initializeAfterTaskCreation();
}

ReturnValue_t TestController::checkModeCommand(Mode_t mode, Submode_t submode,
                                               uint32_t *msToReachTheMode) {
  return returnvalue::OK;
}
