#include <framework/osal/linux/TcUnixUdpPollingTask.h>

TcSocketPollingTask::TcSocketPollingTask(object_id_t objectId,
		object_id_t tmtcUnixUdpBridge): SystemObject(objectId) {
}

TcSocketPollingTask::~TcSocketPollingTask() {
}

ReturnValue_t TcSocketPollingTask::performOperation(uint8_t opCode) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TcSocketPollingTask::initialize() {
	return HasReturnvaluesIF::RETURN_OK;
}
