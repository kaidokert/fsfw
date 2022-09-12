#include "FsfwFactory.h"
#include <OBSWConfig.h>

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/internalerror/InternalErrorReporter.h>

#include <cstdint>

/**
 * This function builds all system objects required for using
 * the FSFW. It is recommended to build all other required objects
 * in a function with an identical prototype, call this function in it and
 * then pass the function to the object manager so it builds all system
 * objects on software startup.
 *
 * All system objects are registered in the internal object manager
 * automatically. The objects should be added to tasks at a later stage, using
 * their objects IDs.
 *
 * This function also sets static framework IDs.
 *
 * Framework should be created first before creating mission system objects.
 * @ingroup init
 */
void Factory::produceFsfwObjects(void) {
	setStaticFrameworkObjectIds();
	new EventManager(objects::EVENT_MANAGER);
	new HealthTable(objects::HEALTH_TABLE);
	new InternalErrorReporter(objects::INTERNAL_ERROR_REPORTER);
}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::PUS_DISTRIBUTOR = objects::NO_OBJECT;
	PusServiceBase::PACKET_DESTINATION = objects::NO_OBJECT;

	CommandingServiceBase::defaultPacketSource = objects::NO_OBJECT;
	CommandingServiceBase::defaultPacketDestination = objects::NO_OBJECT;

	DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;

	DeviceHandlerFailureIsolation::powerConfirmationId = objects::NO_OBJECT;
}

