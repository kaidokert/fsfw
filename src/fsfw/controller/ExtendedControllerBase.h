#ifndef FSFW_CONTROLLER_EXTENDEDCONTROLLERBASE_H_
#define FSFW_CONTROLLER_EXTENDEDCONTROLLERBASE_H_

#include "ControllerBase.h"
#include "fsfw/action.h"
#include "fsfw/datapoollocal/HasLocalDataPoolIF.h"
#include "fsfw/datapoollocal/LocalDataPoolManager.h"

/**
 * @brief   Extends the basic ControllerBase with commonly used components
 * @details
 * HasActionsIF for commandability and HasLocalDataPoolIF to keep a pool of local data pool
 * variables. Default implementations required for the interfaces will be empty and have
 * to be implemented by child class.
 */
class ExtendedControllerBase : public ControllerBase,
                               public HasActionsIF,
                               public HasLocalDataPoolIF {
 public:
  ExtendedControllerBase(object_id_t objectId, object_id_t parentId, size_t commandQueueDepth = 3);
  ~ExtendedControllerBase() override;

  /* SystemObjectIF overrides */
  ReturnValue_t initialize() override;

  [[nodiscard]] MessageQueueId_t getCommandQueue() const override;

  /* ExecutableObjectIF overrides */
  ReturnValue_t performOperation(uint8_t opCode) override;
  ReturnValue_t initializeAfterTaskCreation() override;

 protected:
  LocalDataPoolManager poolManager;
  ActionHelper actionHelper;

  /**
   * Implemented by child class. Handle all command messages which are
   * not health, mode, action or housekeeping messages.
   * @param message
   * @return
   */
  ReturnValue_t handleCommandMessage(CommandMessage* message) override = 0;

  /**
   * Periodic helper from ControllerBase, implemented by child class.
   */
  void performControlOperation() override = 0;

  /* Handle the four messages mentioned above */
  void handleQueue() override;

  /* HasActionsIF overrides */
  ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
                              const uint8_t* data, size_t size) override;

  /* HasLocalDatapoolIF overrides */
  LocalDataPoolManager* getHkManagerHandle() override;
  [[nodiscard]] object_id_t getObjectId() const override;
  [[nodiscard]] uint32_t getPeriodicOperationFrequency() const override;

  ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                        LocalDataPoolManager& poolManager) override = 0;
  LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override = 0;

  // Mode abstract functions
  ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                 uint32_t* msToReachTheMode) override = 0;
};

#endif /* FSFW_CONTROLLER_EXTENDEDCONTROLLERBASE_H_ */
