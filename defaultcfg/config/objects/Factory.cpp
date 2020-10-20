#include "Factory.h"
#include "../tmtc/apid.h"
#include "../tmtc/pusIds.h"
#include "../objects/systemObjectList.h"
#include "../devices/logicalAddresses.h"
#include "../devices/powerSwitcherList.h"

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/events/EventManager.h>
#include <fsfw/health/HealthTable.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcservices/PusServiceBase.h>

#include <cstdint>

/**
 * This class should be used to create all system objects required for
 * the on-board software, using the object ID list from the configuration
 * folder.
 *
 * The objects are registered in the internal object manager automatically.
 * This is used later to add objects to tasks.
 *
 * This file also sets static framework IDs.
 *
 * Framework objects are created first.
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
	//TmFunnel::downlinkDestination = objects::NO_OBJECT;
}

