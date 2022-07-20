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
  virtual ~ExtendedControllerBase();

  /* SystemObjectIF overrides */
  virtual ReturnValue_t initialize() override;

  virtual MessageQueueId_t getCommandQueue() const override;

  /* ExecutableObjectIF overrides */
  virtual ReturnValue_t performOperation(uint8_t opCode) override;
  virtual ReturnValue_t initializeAfterTaskCreation() override;

 protected:
  LocalDataPoolManager poolManager;
  ActionHelper actionHelper;

  /**
   * Implemented by child class. Handle all command messages which are
   * not health, mode, action or housekeeping messages.
   * @param message
   * @return
   */
  virtual ReturnValue_t handleCommandMessage(CommandMessage* message) = 0;

  /**
   * Periodic helper from ControllerBase, implemented by child class.
   */
  virtual void performControlOperation() = 0;

  /* Handle the four messages mentioned above */
  void handleQueue() override;

  /* HasActionsIF overrides */
  virtual ReturnValue_t executeAction(Action* actionId) override;

  /* HasLocalDatapoolIF overrides */
  virtual LocalDataPoolManager* getHkManagerHandle() override;
  virtual object_id_t getObjectId() const override;
  virtual uint32_t getPeriodicOperationFrequency() const override;

  virtual ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                                LocalDataPoolManager& poolManager) override = 0;
  virtual LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override = 0;

  // Mode abstract functions
  virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                         uint32_t* msToReachTheMode) override = 0;
};

#endif /* FSFW_CONTROLLER_EXTENDEDCONTROLLERBASE_H_ */
