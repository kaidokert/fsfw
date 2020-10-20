#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>

#include <fsfw/internalError/InternalErrorReporter.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/unittest/config/cdatapool/dataPoolInit.h>
#include <fsfw/unittest/config/objects/Factory.h>

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
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER,
				datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
				datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
				datapool::INTERNAL_ERROR_STORE_FULL);

}

void Factory::setStaticFrameworkObjectIds() {

}


