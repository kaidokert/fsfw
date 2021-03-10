#ifndef FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_
#define FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_

#include <testcfg/objects/systemObjectList.h>

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/LocalDataSet.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/LocalPoolVector.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/unittest/tests/mocks/MessageQueueMockBase.h>
#include <fsfw/datapool/PoolReadGuard.h>

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
}


class LocalPoolStaticTestDataSet: public StaticLocalDataSet<3> {
public:
    LocalPoolStaticTestDataSet():
            StaticLocalDataSet(lpool::testSid) {
    }

    LocalPoolStaticTestDataSet(HasLocalDataPoolIF* owner, uint32_t setId):
            StaticLocalDataSet(owner, setId) {
    }

    lp_var_t<uint8_t> localPoolVarUint8 = lp_var_t<uint8_t>(lpool::uint8VarGpid, this);
    lp_var_t<float> localPoolVarFloat = lp_var_t<float>(lpool::floatVarGpid, this);
    lp_vec_t<uint16_t, 3> localPoolUint16Vec = lp_vec_t<uint16_t, 3>(lpool::uint16Vec3Gpid, this);

private:
};

class LocalPoolTestDataSet: public LocalDataSet {
public:
    LocalPoolTestDataSet():
        LocalDataSet(lpool::testSid, lpool::dataSetMaxVariables) {
    }

    LocalPoolTestDataSet(HasLocalDataPoolIF* owner, uint32_t setId):
        LocalDataSet(owner, setId, lpool::dataSetMaxVariables) {
    }

    lp_var_t<uint8_t> localPoolVarUint8 = lp_var_t<uint8_t>(lpool::uint8VarGpid, this);
    lp_var_t<float> localPoolVarFloat = lp_var_t<float>(lpool::floatVarGpid, this);
    lp_vec_t<uint16_t, 3> localPoolUint16Vec = lp_vec_t<uint16_t, 3>(lpool::uint16Vec3Gpid, this);

private:
};


class LocalPoolOwnerBase: public SystemObject, public HasLocalDataPoolIF {
public:
    LocalPoolOwnerBase(object_id_t objectId = objects::TEST_LOCAL_POOL_OWNER_BASE);

    ~LocalPoolOwnerBase() {
        QueueFactory::instance()->deleteMessageQueue(messageQueue);
    }

    object_id_t getObjectId() const override {
        return SystemObject::getObjectId();
    }

    ReturnValue_t initializeHkManager();

    ReturnValue_t initializeHkManagerAfterTaskCreation();

    /** Command queue for housekeeping messages. */
    MessageQueueId_t getCommandQueue() const override {
        return messageQueue->getId();
    }

    // This is called by initializeAfterTaskCreation of the HK manager.
    virtual ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
            LocalDataPoolManager& poolManager) override;

    LocalDataPoolManager* getHkManagerHandle() override {
        return &poolManager;
    }

    dur_millis_t getPeriodicOperationFrequency() const override {
        return 200;
    }

    /**
     * This function is used by the pool manager to get a valid dataset
     * from a SID
     * @param sid Corresponding structure ID
     * @return
     */
    virtual LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override  {
        return &dataset;
    }

    virtual LocalPoolObjectBase* getPoolObjectHandle(lp_id_t localPoolId) override;

    MessageQueueMockBase* getMockQueueHandle() const {
        return dynamic_cast<MessageQueueMockBase*>(messageQueue);
    }

    ReturnValue_t subscribePeriodicHk(bool enableReporting) {
        return poolManager.subscribeForPeriodicPacket(lpool::testSid, enableReporting, 0.2, false);
    }

    ReturnValue_t subscribeWrapperSetUpdate() {
        return poolManager.subscribeForSetUpdateMessage(lpool::testSetId,
                objects::NO_OBJECT, objects::HK_RECEIVER_MOCK, false);
    }

    ReturnValue_t subscribeWrapperSetUpdateSnapshot() {
        return poolManager.subscribeForSetUpdateMessage(lpool::testSetId,
                objects::NO_OBJECT, objects::HK_RECEIVER_MOCK, true);
    }

    ReturnValue_t subscribeWrapperSetUpdateHk(bool diagnostics = false) {
        return poolManager.subscribeForUpdatePacket(lpool::testSid, diagnostics,
                false, objects::HK_RECEIVER_MOCK);
    }

    ReturnValue_t subscribeWrapperVariableUpdate(lp_id_t localPoolId) {
        return poolManager.subscribeForVariableUpdateMessage(localPoolId,
                MessageQueueIF::NO_QUEUE, objects::HK_RECEIVER_MOCK, false);
    }

    ReturnValue_t subscribeWrapperVariableSnapshot(lp_id_t localPoolId) {
        return poolManager.subscribeForVariableUpdateMessage(localPoolId,
                MessageQueueIF::NO_QUEUE, objects::HK_RECEIVER_MOCK, true);
    }

    ReturnValue_t reset();

    void resetSubscriptionList() {
        poolManager.clearReceiversList();
    }

    bool changedDataSetCallbackWasCalled(sid_t& sid, store_address_t& storeId);
    bool changedVariableCallbackWasCalled(gp_id_t& gpid, store_address_t& storeId);

    LocalDataPoolManager poolManager;
    LocalPoolTestDataSet dataset;
private:

    void handleChangedDataset(sid_t sid, store_address_t storeId) override;
    sid_t changedDatasetSid;
    store_address_t storeIdForChangedSet;

    void handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId) override;
    gp_id_t changedPoolVariableGpid;
    store_address_t storeIdForChangedVariable;

    lp_var_t<uint8_t> testUint8 = lp_var_t<uint8_t>(this, lpool::uint8VarId);
    lp_var_t<float> testFloat = lp_var_t<float>(this, lpool::floatVarId);
    lp_var_t<uint32_t> testUint32 = lp_var_t<uint32_t>(this, lpool::uint32VarId);

    lp_vec_t<uint16_t, 3> testUint16Vec = lp_vec_t<uint16_t, 3>(this,
            lpool::uint16Vec3Id);
    lp_vec_t<int64_t, 2> testInt64Vec = lp_vec_t<int64_t, 2>(this,
            lpool::int64Vec2Id);

    MessageQueueIF* messageQueue = nullptr;

    bool initialized = false;
    bool initializedAfterTaskCreation = false;

};

#endif /* FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_ */
