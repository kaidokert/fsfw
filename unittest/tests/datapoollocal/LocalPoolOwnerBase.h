#ifndef FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_
#define FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/LocalDataSet.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/LocalPoolVector.h>
#include <fsfw/ipc/QueueFactory.h>
#include <testcfg/objects/systemObjectList.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/unittest/tests/mocks/MessageQueueMockBase.h>

namespace lpool {
static constexpr lp_id_t uint8VarId = 0;
static constexpr lp_id_t floatVarId = 1;
static constexpr lp_id_t uint32VarId = 2;
static constexpr lp_id_t uint16Vec3Id = 3;
static constexpr lp_id_t int64Vec2Id = 4;

static constexpr uint32_t testSetId = 0;
static constexpr uint8_t dataSetMaxVariables = 10;
static const sid_t testSid = sid_t(objects::TEST_LOCAL_POOL_OWNER_BASE,
		testSetId);
}


class LocalPoolTestDataSet: public LocalDataSet {
public:
	LocalPoolTestDataSet(HasLocalDataPoolIF* owner, uint32_t setId):
		LocalDataSet(owner, setId, lpool::dataSetMaxVariables) {
	}

	ReturnValue_t assignPointers() {
		PoolVariableIF** rawVarArray = getContainer();
		localPoolVarUint8 = dynamic_cast<lp_var_t<uint8_t>*>(rawVarArray[0]);
		localPoolVarFloat = dynamic_cast<lp_var_t<float>*>(rawVarArray[1]);
		localPoolUint16Vec = dynamic_cast<lp_vec_t<uint16_t, 3>*>(
				rawVarArray[2]);
		if(localPoolVarUint8 == nullptr or  localPoolVarFloat == nullptr or
				localPoolUint16Vec == nullptr) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	lp_var_t<uint8_t>* localPoolVarUint8 = nullptr;
	lp_var_t<float>* localPoolVarFloat = nullptr;
	lp_vec_t<uint16_t, 3>* localPoolUint16Vec = nullptr;

private:
};


class LocalPoolOwnerBase: public SystemObject, public HasLocalDataPoolIF {
public:
	LocalPoolOwnerBase(
			object_id_t objectId = objects::TEST_LOCAL_POOL_OWNER_BASE):
		SystemObject(objectId), hkManager(this, messageQueue),
		dataset(this, lpool::testSetId) {
		messageQueue = new MessageQueueMockBase();
	}

	~LocalPoolOwnerBase() {
		QueueFactory::instance()->deleteMessageQueue(messageQueue);
	}

	object_id_t getObjectId() const override {
		return SystemObject::getObjectId();
	}

	ReturnValue_t initializeHkManager() {
		if(not initialized) {
			initialized = true;
			return hkManager.initialize(messageQueue);
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t initializeHkManagerAfterTaskCreation() {
		if(not initializedAfterTaskCreation) {
			initializedAfterTaskCreation = true;
			return hkManager.initializeAfterTaskCreation();
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	/** Command queue for housekeeping messages. */
	MessageQueueId_t getCommandQueue() const override {
		return messageQueue->getId();
	}

	// This is called by initializeAfterTaskCreation of the HK manager.
	virtual ReturnValue_t initializeLocalDataPool(
	        LocalDataPool& localDataPoolMap,
	        LocalDataPoolManager& poolManager) {
		// Default initialization empty for now.
		localDataPoolMap.emplace(lpool::uint8VarId,
				new PoolEntry<uint8_t>({0}));
		localDataPoolMap.emplace(lpool::floatVarId,
				new PoolEntry<float>({0}));
		localDataPoolMap.emplace(lpool::uint32VarId,
				new PoolEntry<uint32_t>({0}));

		localDataPoolMap.emplace(lpool::uint16Vec3Id,
				new PoolEntry<uint16_t>({0, 0, 0}));
		localDataPoolMap.emplace(lpool::int64Vec2Id,
				new PoolEntry<int64_t>({0, 0}));
		return HasReturnvaluesIF::RETURN_OK;
	}

	LocalDataPoolManager* getHkManagerHandle() override {
		return &hkManager;
	}

	uint32_t getPeriodicOperationFrequency() const override {
		return 0;
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

	virtual LocalPoolObjectBase* getPoolObjectHandle(
			lp_id_t localPoolId) override {
		if(localPoolId == lpool::uint8VarId) {
			return &testUint8;
		}
		else if(localPoolId == lpool::uint16Vec3Id) {
			return &testUint16Vec;
		}
		else if(localPoolId == lpool::floatVarId) {
			return &testFloat;
		}
		else if(localPoolId == lpool::int64Vec2Id) {
			return &testInt64Vec;
		}
		else if(localPoolId == lpool::uint32VarId) {
			return &testUint32;
		}
		else {
			return &testUint8;
		}
	}

	MessageQueueMockBase* getMockQueueHandle() const {
		return dynamic_cast<MessageQueueMockBase*>(messageQueue);
	}

	ReturnValue_t subscribeWrapperSetUpdate() {
		return hkManager.subscribeForSetUpdateMessages(lpool::testSetId,
				objects::NO_OBJECT, MessageQueueIF::NO_QUEUE, false);
	}

	ReturnValue_t subscribeWrapperSetUpdateHk(bool diagnostics = false) {
		return hkManager.subscribeForUpdatePackets(lpool::testSid, diagnostics,
				false, objects::HK_RECEIVER_MOCK);
	}

	ReturnValue_t subscribeWrapperVariableUpdate(lp_id_t localPoolId) {
		return hkManager.subscribeForVariableUpdateMessages(localPoolId,
				MessageQueueIF::NO_QUEUE, objects::NO_OBJECT, false);
	}

	LocalDataPoolManager hkManager;
	LocalPoolTestDataSet dataset;
private:

	lp_var_t<uint8_t> testUint8 = lp_var_t<uint8_t>(this, lpool::uint8VarId,
			&dataset);
	lp_var_t<float> testFloat = lp_var_t<float>(this, lpool::floatVarId,
			&dataset);
	lp_var_t<uint32_t> testUint32 = lp_var_t<uint32_t>(this, lpool::uint32VarId);

	lp_vec_t<uint16_t, 3> testUint16Vec = lp_vec_t<uint16_t, 3>(this,
			lpool::uint16Vec3Id, &dataset);
	lp_vec_t<int64_t, 2> testInt64Vec = lp_vec_t<int64_t, 2>(this,
			lpool::int64Vec2Id);

	MessageQueueIF* messageQueue = nullptr;

	bool initialized = false;
	bool initializedAfterTaskCreation = false;

};

#endif /* FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_ */
