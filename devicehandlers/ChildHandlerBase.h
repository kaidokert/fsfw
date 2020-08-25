#ifndef PAYLOADHANDLERBASE_H_
#define PAYLOADHANDLERBASE_H_

#include "ChildHandlerFDIR.h"
#include "DeviceHandlerBase.h"

class ChildHandlerBase: public DeviceHandlerBase {
public:
	ChildHandlerBase(object_id_t setObjectId,
			object_id_t deviceCommunication, CookieIF * comCookie,
			uint8_t setDeviceSwitch, uint32_t thermalStatePoolId,
			uint32_t thermalRequestPoolId, uint32_t parent,
			FailureIsolationBase* customFdir = nullptr,
			size_t cmdQueueSize = 20);
	virtual ~ChildHandlerBase();

	virtual ReturnValue_t initialize();

protected:
	const uint32_t parentId;
	ChildHandlerFDIR childHandlerFdir;

};

#endif /* PAYLOADHANDLERBASE_H_ */
