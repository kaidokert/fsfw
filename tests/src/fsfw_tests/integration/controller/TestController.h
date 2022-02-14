#ifndef MISSION_CONTROLLER_TESTCONTROLLER_H_
#define MISSION_CONTROLLER_TESTCONTROLLER_H_

#include <fsfw/controller/ExtendedControllerBase.h>

#include "../devices/devicedefinitions/testDeviceDefinitions.h"

class TestController : public ExtendedControllerBase {
 public:
  TestController(object_id_t objectId, object_id_t parentId, size_t commandQueueDepth = 10);
  virtual ~TestController();

 protected:
  // Extended Controller Base overrides
  ReturnValue_t handleCommandMessage(CommandMessage* message) override;
  void performControlOperation() override;

  // HasLocalDatapoolIF callbacks
  virtual void handleChangedDataset(sid_t sid, store_address_t storeId,
                                    bool* clearMessage) override;
  virtual void handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
                                         bool* clearMessage) override;

  LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override;
  ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                        LocalDataPoolManager& poolManager) override;

  ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                 uint32_t* msToReachTheMode) override;

  ReturnValue_t initializeAfterTaskCreation() override;

 private:
};

#endif /* MISSION_CONTROLLER_TESTCONTROLLER_H_ */
