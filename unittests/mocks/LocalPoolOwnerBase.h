#ifndef FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_
#define FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_

#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/LocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/LocalPoolVector.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/objectmanager/SystemObject.h>

#include "fsfw/datapool/PoolEntry.h"
#include "mocks/MessageQueueMock.h"
#include "tests/TestsConfig.h"

namespace lpool {
static constexpr lp_id_t uint8VarId = 0;
static constexpr lp_id_t floatVarId = 1;
static constexpr lp_id_t uint32VarId = 2;
static constexpr lp_id_t uint16Vec3Id = 3;
static constexpr lp_id_t int64Vec2Id = 4;

static constexpr uint32_t testSetId = 0;
static constexpr uint8_t dataSetMaxVariables = 10;

static const sid_t testSid = sid_t(objects::TEST_LOCAL_POOL_OWNER_BASE, testSetId);

static const gp_id_t uint8VarGpid = gp_id_t(objects::TEST_LOCAL_POOL_OWNER_BASE, uint8VarId);
static const gp_id_t floatVarGpid = gp_id_t(objects::TEST_LOCAL_POOL_OWNER_BASE, floatVarId);
static const gp_id_t uint32Gpid = gp_id_t(objects::TEST_LOCAL_POOL_OWNER_BASE, uint32VarId);
static const gp_id_t uint16Vec3Gpid = gp_id_t(objects::TEST_LOCAL_POOL_OWNER_BASE, uint16Vec3Id);
static const gp_id_t uint64Vec2Id = gp_id_t(objects::TEST_LOCAL_POOL_OWNER_BASE, int64Vec2Id);
}  // namespace lpool

class LocalPoolStaticTestDataSet : public StaticLocalDataSet<3> {
 public:
  LocalPoolStaticTestDataSet() : StaticLocalDataSet(lpool::testSid) {}

  LocalPoolStaticTestDataSet(HasLocalDataPoolIF* owner, uint32_t setId)
      : StaticLocalDataSet(owner, setId) {}

  lp_var_t<uint8_t> localPoolVarUint8 = lp_var_t<uint8_t>(lpool::uint8VarGpid, this);
  lp_var_t<float> localPoolVarFloat = lp_var_t<float>(lpool::floatVarGpid, this);
  lp_vec_t<uint16_t, 3> localPoolUint16Vec = lp_vec_t<uint16_t, 3>(lpool::uint16Vec3Gpid, this);

 private:
};

class LocalPoolTestDataSet : public LocalDataSet {
 public:
  LocalPoolTestDataSet() : LocalDataSet(lpool::testSid, lpool::dataSetMaxVariables) {}

  LocalPoolTestDataSet(HasLocalDataPoolIF* owner, uint32_t setId)
      : LocalDataSet(owner, setId, lpool::dataSetMaxVariables) {}

  lp_var_t<uint8_t> localPoolVarUint8 = lp_var_t<uint8_t>(lpool::uint8VarGpid, this);
  lp_var_t<float> localPoolVarFloat = lp_var_t<float>(lpool::floatVarGpid, this);
  lp_vec_t<uint16_t, 3> localPoolUint16Vec = lp_vec_t<uint16_t, 3>(lpool::uint16Vec3Gpid, this);

  void setDiagnostic(bool isDiagnostic) { LocalPoolDataSetBase::setDiagnostic(isDiagnostic); }

 private:
};

class LocalPoolOwnerBase : public SystemObject, public HasLocalDataPoolIF {
 public:
  explicit LocalPoolOwnerBase(MessageQueueIF& queue,
                              object_id_t objectId = objects::TEST_LOCAL_POOL_OWNER_BASE);

  ~LocalPoolOwnerBase() override;

  [[nodiscard]] object_id_t getObjectId() const override { return SystemObject::getObjectId(); }

  ReturnValue_t initializeHkManager();

  void setHkDestId(MessageQueueId_t id);

  ReturnValue_t initializeHkManagerAfterTaskCreation();

  /** Command queue for housekeeping messages. */
  [[nodiscard]] MessageQueueId_t getCommandQueue() const override { return queue.getId(); }

  // This is called by initializeAfterTaskCreation of the HK manager.
  ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                        LocalDataPoolManager& poolManager) override;

  LocalDataPoolManager* getHkManagerHandle() override { return &poolManager; }

  [[nodiscard]] dur_millis_t getPeriodicOperationFrequency() const override { return 200; }

  /**
   * This function is used by the pool manager to get a valid dataset
   * from a SID
   * @param sid Corresponding structure ID
   * @return
   */
  LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override { return &dataset; }

  LocalPoolObjectBase* getPoolObjectHandle(lp_id_t localPoolId) override;

  [[nodiscard]] MessageQueueMock& getMockQueueHandle() const {
    return dynamic_cast<MessageQueueMock&>(queue);
  }

  ReturnValue_t subscribePeriodicHk(bool enableReporting) {
    return poolManager.subscribeForRegularPeriodicPacket(
        subdp::RegularHkPeriodicParams(lpool::testSid, enableReporting, 0.2));
  }

  ReturnValue_t subscribeWrapperSetUpdate(MessageQueueId_t receiverId) {
    return poolManager.subscribeForSetUpdateMessage(lpool::testSetId, objects::NO_OBJECT,
                                                    receiverId, false);
  }

  ReturnValue_t subscribeWrapperSetUpdateSnapshot(MessageQueueId_t receiverId) {
    return poolManager.subscribeForSetUpdateMessage(lpool::testSetId, objects::NO_OBJECT,
                                                    receiverId, true);
  }

  ReturnValue_t subscribeWrapperSetUpdateHk(bool diagnostics = false,
                                            AcceptsHkPacketsIF* receiver = nullptr) {
    if (diagnostics) {
      auto params = subdp::DiagnosticsHkUpdateParams(lpool::testSid, true);
      if (receiver != nullptr) {
        params.receiver = receiver->getHkQueue();
      }
      return poolManager.subscribeForDiagUpdatePacket(params);
    } else {
      auto params = subdp::RegularHkUpdateParams(lpool::testSid, true);
      if (receiver != nullptr) {
        params.receiver = receiver->getHkQueue();
      }
      return poolManager.subscribeForRegularUpdatePacket(params);
    }
  }

  ReturnValue_t subscribeWrapperVariableUpdate(MessageQueueId_t receiverId, lp_id_t localPoolId) {
    return poolManager.subscribeForVariableUpdateMessage(localPoolId, MessageQueueIF::NO_QUEUE,
                                                         receiverId, false);
  }

  ReturnValue_t subscribeWrapperVariableSnapshot(MessageQueueId_t receiverId, lp_id_t localPoolId) {
    return poolManager.subscribeForVariableUpdateMessage(localPoolId, MessageQueueIF::NO_QUEUE,
                                                         receiverId, true);
  }

  ReturnValue_t reset();

  void resetSubscriptionList() { poolManager.clearReceiversList(); }

  bool changedDataSetCallbackWasCalled(sid_t& sid, store_address_t& storeId);
  bool changedVariableCallbackWasCalled(gp_id_t& gpid, store_address_t& storeId);

  LocalDataPoolManager poolManager;
  LocalPoolTestDataSet dataset;

 private:
  void handleChangedDataset(sid_t sid, store_address_t storeId, bool* clearMessage) override;
  sid_t changedDatasetSid;
  store_address_t storeIdForChangedSet;

  void handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
                                 bool* clearMessage) override;
  gp_id_t changedPoolVariableGpid;
  store_address_t storeIdForChangedVariable;

  PoolEntry<uint8_t> u8PoolEntry = PoolEntry<uint8_t>({0});
  PoolEntry<float> floatPoolEntry = PoolEntry<float>({0});
  PoolEntry<uint32_t> u32PoolEntry = PoolEntry<uint32_t>({0});
  PoolEntry<uint16_t> u16VecPoolEntry = PoolEntry<uint16_t>({0, 0, 0});
  PoolEntry<int64_t> i64VecPoolEntry = PoolEntry<int64_t>({0, 0});

  lp_var_t<uint8_t> testUint8 = lp_var_t<uint8_t>(this, lpool::uint8VarId);
  lp_var_t<float> testFloat = lp_var_t<float>(this, lpool::floatVarId);
  lp_var_t<uint32_t> testUint32 = lp_var_t<uint32_t>(this, lpool::uint32VarId);
  lp_vec_t<uint16_t, 3> testUint16Vec = lp_vec_t<uint16_t, 3>(this, lpool::uint16Vec3Id);
  lp_vec_t<int64_t, 2> testInt64Vec = lp_vec_t<int64_t, 2>(this, lpool::int64Vec2Id);

  MessageQueueIF& queue;

  bool initialized = false;
  bool initializedAfterTaskCreation = false;
};

#endif /* FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_ */
