#ifndef PAYLOADHANDLERBASE_H_
#define PAYLOADHANDLERBASE_H_

#include <framework/devicehandlers/ChildHandlerFDIR.h>
#include <framework/devicehandlers/DeviceHandlerBase.h>

class ChildHandlerBase: public DeviceHandlerBase {
public:
	ChildHandlerBase(uint32_t ioBoardAddress, object_id_t setObjectId,
			object_id_t deviceCommunication, uint32_t maxDeviceReplyLen,
			uint8_t setDeviceSwitch, uint32_t thermalStatePoolId,
			uint32_t thermalRequestPoolId, uint32_t parent,
			FailureIsolationBase* customFdir = NULL,
			uint32_t cmdQueueSize = 20);
	virtual ~ChildHandlerBase();

	virtual ReturnValue_t initialize();

protected:
	const uint32_t parentId;
	ChildHandlerFDIR childHandlerFdir;

};

#endif /* PAYLOADHANDLERBASE_H_ */
