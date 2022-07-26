#include "CatchFactory.h"

#include <fsfw/datapoollocal/LocalDataPoolManager.h>
#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/internalerror/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/storagemanager/PoolManager.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcservices/PusServiceBase.h>

#include "mocks/HkReceiverMock.h"
#include "tests/TestsConfig.h"

#if FSFW_ADD_DEFAULT_FACTORY_FUNCTIONS == 1

/**
 * @brief Produces system objects.
 * @details
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the interface validity is checked or retrieved later by using the
 * get<TargetInterface>(object_id) function from the ObjectManagerIF.
 *
 * Framework objects are created first.
 *
 * @ingroup init
 */
void Factory::produceFrameworkObjects(void* args) {
  setStaticFrameworkObjectIds();
  new EventManager(objects::EVENT_MANAGER);
  new HealthTable(objects::HEALTH_TABLE);
  new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

  {
    PoolManager::LocalPoolConfig poolCfg = {{100, 16}, {50, 32}, {25, 64}, {15, 128}, {5, 1024}};
    new PoolManager(objects::TM_STORE, poolCfg);
  }

  {
    PoolManager::LocalPoolConfig poolCfg = {{100, 16}, {50, 32}, {25, 64}, {15, 128}, {5, 1024}};
    new PoolManager(objects::IPC_STORE, poolCfg);
  }
}

// TODO: Our tests, and the code base in general should really not depend on some arbitrary function
//       like this. Instead, this should be more like a general struct containing all important
//       object IDs which are then explicitely passed in the object constructor
void Factory::setStaticFrameworkObjectIds() {
  PusServiceBase::PACKET_DESTINATION = objects::NO_OBJECT;

  CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
  CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

  DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
  DeviceHandlerBase::rawDataReceiverId = objects::NO_OBJECT;

  LocalDataPoolManager::defaultHkDestination = objects::NO_OBJECT;
  DeviceHandlerFailureIsolation::powerConfirmationId = objects::NO_OBJECT;
}

#endif
