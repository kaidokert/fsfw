#ifndef MISSION_CONTROLLER_TESTCONTROLLER_H_
#define MISSION_CONTROLLER_TESTCONTROLLER_H_

#include "../devices/devicedefinitions/testDeviceDefinitions.h"
#include <fsfw/controller/ExtendedControllerBase.h>


class TestController:
        public ExtendedControllerBase {
public:
    TestController(object_id_t objectId, object_id_t device0, object_id_t device1,
            size_t commandQueueDepth = 10);
    virtual~ TestController();
protected:
    testdevice::TestDataSet deviceDataset0;
    testdevice::TestDataSet deviceDataset1;

    /* Extended Controller Base overrides */
    ReturnValue_t handleCommandMessage(CommandMessage *message) override;
    void performControlOperation() override;

    /* HasLocalDatapoolIF callbacks */
    void handleChangedDataset(sid_t sid, store_address_t storeId, bool* clearMessage) override;
    void handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
            bool* clearMessage) override;

    LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override;
    ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                LocalDataPoolManager& poolManager) override;

    ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
            uint32_t *msToReachTheMode) override;

    ReturnValue_t initializeAfterTaskCreation() override;

private:

    bool traceVariable = false;
    uint8_t traceCycles = 5;
    uint8_t traceCounter = traceCycles;

    enum TraceTypes {
        NONE,
        TRACE_DEV_0_UINT8,
        TRACE_DEV_0_VECTOR
    };
    TraceTypes currentTraceType = TraceTypes::NONE;
};


#endif /* MISSION_CONTROLLER_TESTCONTROLLER_H_ */
