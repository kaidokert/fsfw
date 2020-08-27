#ifndef PAYLOADHANDLERBASE_H_
#define PAYLOADHANDLERBASE_H_

#include "../devicehandlers/ChildHandlerFDIR.h"
#include "../devicehandlers/DeviceHandlerBase.h"

class ChildHandlerBase: public DeviceHandlerBase {
public:
	ChildHandlerBase(object_id_t setObjectId, object_id_t deviceCommunication,
			CookieIF * cookie, uint32_t thermalStatePoolId,
			uint32_t thermalRequestPoolId, object_id_t parent = objects::NO_OBJECT,
			FailureIsolationBase* customFdir = nullptr, size_t cmdQueueSize = 20);
	virtual ~ChildHandlerBase();

	virtual ReturnValue_t initialize();

protected:
	const uint32_t parentId;
	ChildHandlerFDIR childHandlerFdir;

};

#endif /* PAYLOADHANDLERBASE_H_ */
