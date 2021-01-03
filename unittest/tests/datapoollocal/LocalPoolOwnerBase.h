#ifndef FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_
#define FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/ipc/QueueFactory.h>
#include <testcfg/objects/systemObjectList.h>

namespace lpool {
static constexpr lp_id_t uint8VarId = 0;
static constexpr lp_id_t floatVarId = 1;
}


class LocalPoolOwnerBase: public SystemObject, public HasLocalDataPoolIF {
public:
	LocalPoolOwnerBase(
			object_id_t objectId = objects::TEST_LOCAL_POOL_OWNER_BASE):
		SystemObject(objectId), hkManager(this, messageQueue)  {
		messageQueue = QueueFactory::instance()->createMessageQueue(10);
	}

	~LocalPoolOwnerBase() {
		QueueFactory::instance()->deleteMessageQueue(messageQueue);
	}

	object_id_t getObjectId() const override {
		return SystemObject::getObjectId();
	}

	ReturnValue_t initializeHkManager() {
		return hkManager.initialize(messageQueue);

	}

	ReturnValue_t initializeHkManagerAfterTaskCreation() {
		return hkManager.initializeAfterTaskCreation();
	}

	/** Command queue for housekeeping messages. */
	MessageQueueId_t getCommandQueue() const override {
		return messageQueue->getId();
	}

	virtual ReturnValue_t initializeLocalDataPool(
	        LocalDataPool& localDataPoolMap,
	        LocalDataPoolManager& poolManager) {
		// Default initialization empty for now.
		localDataPoolMap.emplace(lpool::uint8VarId, new PoolEntry<uint8_t>({0}));
		localDataPoolMap.emplace(lpool::floatVarId, new PoolEntry<float>({0}));
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
		// empty for now
		return nullptr;
	}
private:

	lp_var_t<uint8_t> testUint8 = lp_var_t<uint8_t>(this, lpool::uint8VarId);
	lp_var_t<float> testFloat = lp_var_t<float>(this, lpool::floatVarId);

	MessageQueueIF* messageQueue = nullptr;
	LocalDataPoolManager hkManager;

};

#endif /* FSFW_UNITTEST_TESTS_DATAPOOLLOCAL_LOCALPOOLOWNERBASE_H_ */
