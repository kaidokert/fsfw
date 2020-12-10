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
	//new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);

	{
		static constexpr uint8_t NUMBER_OF_POOLS = 5;
		const uint16_t element_sizes[NUMBER_OF_POOLS] = {16, 32, 64, 128, 1024};
		const uint16_t n_elements[NUMBER_OF_POOLS] = {100, 50, 25, 15, 5};
		new PoolManager<NUMBER_OF_POOLS>(objects::TC_STORE, element_sizes,
				n_elements);
	}

	{
		static constexpr uint8_t NUMBER_OF_POOLS = 5;
		const uint16_t element_sizes[NUMBER_OF_POOLS] = {16, 32, 64, 128, 1024};
		const uint16_t n_elements[NUMBER_OF_POOLS] = {100, 50, 25, 15, 5};
		new PoolManager<NUMBER_OF_POOLS>(objects::TM_STORE, element_sizes,
				n_elements);
	}

	{
		static constexpr uint8_t NUMBER_OF_POOLS = 6;
		const uint16_t element_sizes[NUMBER_OF_POOLS] = {32, 64, 512,
				1024, 2048, 4096};
		const uint16_t n_elements[NUMBER_OF_POOLS] = {200, 100, 50, 25, 15, 5};
		new PoolManager<NUMBER_OF_POOLS>(objects::IPC_STORE, element_sizes,
				n_elements);
	}

}

void Factory::setStaticFrameworkObjectIds() {

}


