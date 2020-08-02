#include <framework/subsystem/SubsystemBase.h>
#include <framework/devicehandlers/ChildHandlerBase.h>
#include <framework/subsystem/SubsystemBase.h>

ChildHandlerBase::ChildHandlerBase(object_id_t setObjectId,
		object_id_t deviceCommunication, CookieIF * comCookie,
		uint8_t setDeviceSwitch, uint32_t thermalStatePoolId,
		uint32_t thermalRequestPoolId, uint32_t parent,
		FailureIsolationBase* customFdir, size_t cmdQueueSize) :
		DeviceHandlerBase(setObjectId, deviceCommunication, comCookie,
				setDeviceSwitch, thermalStatePoolId,thermalRequestPoolId,
				(customFdir == nullptr? &childHandlerFdir : customFdir),
				cmdQueueSize),
		parentId(parent), childHandlerFdir(setObjectId) {
}

ChildHandlerBase::~ChildHandlerBase() {
}

ReturnValue_t ChildHandlerBase::initialize() {
	ReturnValue_t result = DeviceHandlerBase::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	MessageQueueId_t parentQueue = 0;

	if (parentId != 0) {
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
