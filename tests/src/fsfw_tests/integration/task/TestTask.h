#ifndef MISSION_DEMO_TESTTASK_H_
#define MISSION_DEMO_TESTTASK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/storagemanager/StorageManagerIF.h>

#include "fsfw/events/Event.h"
#include "events/subsystemIdRanges.h"

/**
 * @brief 	Test class for general C++ testing and any other code which will not be part of the
 *          primary mission software.
 * @details
 * Should not be used for board specific tests. Instead, a derived board test class should be used.
 */
class TestTask :
        public SystemObject,
        public ExecutableObjectIF,
        public HasReturnvaluesIF {
public:
    TestTask(object_id_t objectId, bool periodicPrintout = false, bool periodicEvent = false);
    virtual ~TestTask();
    virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

    static constexpr uint8_t subsystemId = SUBSYSTEM_ID::TEST_TASK_ID;
    static constexpr Event TEST_EVENT = event::makeEvent(subsystemId, 0, severity::INFO);

protected:
    virtual ReturnValue_t performOneShotAction();
    virtual ReturnValue_t performPeriodicAction();
    virtual ReturnValue_t performActionA();
    virtual ReturnValue_t performActionB();

    enum testModes: uint8_t {
        A,
        B
    };

    testModes testMode;
    bool periodicPrinout = false;
    bool periodicEvent = false;

    bool testFlag = false;
    uint8_t counter { 1 };
    uint8_t counterTrigger { 3 };

    void performPusInjectorTest();
    void examplePacketTest();
private:
    static bool oneShotAction;
    static MutexIF* testLock;
    StorageManagerIF* IPCStore;
};

#endif /* TESTTASK_H_ */
