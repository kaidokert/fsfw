#include "CatchFactory.h"

#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>

#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/storagemanager/PoolManager.h>

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
void Factory::produce(void) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

	{
		PoolManager::LocalPoolConfig poolCfg = {
		        {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
		};
		new PoolManager(objects::TC_STORE, poolCfg);
	}

	{
        PoolManager::LocalPoolConfig poolCfg = {
                {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
        };
		new PoolManager(objects::TM_STORE, poolCfg);
	}

	{
        PoolManager::LocalPoolConfig poolCfg = {
                {100, 16}, {50, 32}, {25, 64} , {15, 128}, {5, 1024}
        };
		new PoolManager(objects::IPC_STORE, poolCfg);
	}

}

void Factory::setStaticFrameworkObjectIds() {

}


