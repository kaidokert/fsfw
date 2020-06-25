#include <framework/subsystem/SubsystemBase.h>
#include <framework/devicehandlers/ChildHandlerBase.h>
#include <framework/subsystem/SubsystemBase.h>

ChildHandlerBase::ChildHandlerBase(object_id_t setObjectId,
		object_id_t deviceCommunication, CookieIF * cookie,
		object_id_t hkDestination, uint32_t thermalStatePoolId,
		uint32_t thermalRequestPoolId,
		object_id_t parent,
		FailureIsolationBase* customFdir, size_t cmdQueueSize) :
		DeviceHandlerBase(setObjectId, deviceCommunication, cookie,
			(customFdir == nullptr? &childHandlerFdir : customFdir),
			cmdQueueSize),
		parentId(parent), childHandlerFdir(setObjectId) {
	this->setHkDestination(hkDestination);
	this->setThermalStateRequestPoolIds(thermalStatePoolId,
			thermalRequestPoolId);

}

ChildHandlerBase::~ChildHandlerBase() {
}

ReturnValue_t ChildHandlerBase::initialize() {
	ReturnValue_t result = DeviceHandlerBase::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	MessageQueueId_t parentQueue = 0;

	if (parentId != objects::NO_OBJECT) {
		SubsystemBase *parent = objectManager->get<SubsystemBase>(parentId);
		if (parent == NULL) {
			return RETURN_FAILED;
		}
		parentQueue = parent->getCommandQueue();

		parent->registerChild(getObjectId());
	}

	healthHelper.setParentQueue(parentQueue);

	modeHelper.setParentQueue(parentQueue);

	return RETURN_OK;
}
