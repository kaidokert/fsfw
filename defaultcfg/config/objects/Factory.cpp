#include "Factory.h"

/* Config */
#include <tmtc/apid.h>
#include <objects/systemObjectList.h>
#include <devices/logicalAddresses.h>
#include <devices/powerSwitcherList.h>
#include <tmtc/pusIds.h>

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <mission/utility/TmFunnel.h>


#include <cstdint>


/**
 * Build tasks by using SystemObject Interface (Interface).
 * Header files of all tasks must be included
 * Please note that an object has to implement the system object interface
 * if the nterface validity is checked or retrieved later by using the
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
}

void Factory::setStaticFrameworkObjectIds() {
	PusServiceBase::packetSource = objects::PUS_PACKET_DISTRIBUTOR;
	PusServiceBase::packetDestination = objects::TM_FUNNEL;

	CommandingServiceBase::defaultPacketSource = objects::PUS_PACKET_DISTRIBUTOR;
	CommandingServiceBase::defaultPacketDestination = objects::TM_FUNNEL;

	VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;

	DeviceHandlerBase::powerSwitcherId = objects::NO_OBJECT;
	DeviceHandlerBase::rawDataReceiverId = objects::PUS_SERVICE_2_DEVICE_ACCESS;

	DeviceHandlerFailureIsolation::powerConfirmationId = objects::NO_OBJECT;

	TmPacketStored::timeStamperId = objects::PUS_TIME;
	TmFunnel::downlinkDestination = objects::NO_OBJECT;
}

