#include <framework/subsystem/SubsystemBase.h>
#include <framework/devicehandlers/ChildHandlerBase.h>
#include <framework/subsystem/SubsystemBase.h>

ChildHandlerBase::ChildHandlerBase(uint32_t ioBoardAddress,
		object_id_t setObjectId, object_id_t deviceCommunication,
		uint32_t maxDeviceReplyLen, uint8_t setDeviceSwitch,
		uint32_t thermalStatePoolId, uint32_t thermalRequestPoolId,
		uint32_t parent, FDIRBase* customFdir, uint32_t cmdQueueSize) :
		DeviceHandlerBase(ioBoardAddress, setObjectId, maxDeviceReplyLen,
				setDeviceSwitch, deviceCommunication, thermalStatePoolId,
				thermalRequestPoolId, (customFdir == NULL? &childHandlerFdir : customFdir), cmdQueueSize), parentId(
				parent), childHandlerFdir(setObjectId) {
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

	healthHelper.setParentQeueue(parentQueue);

	modeHelper.setParentQueue(parentQueue);

	return RETURN_OK;
}
