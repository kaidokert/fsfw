#ifndef FSFW_DEVICEHANDLER_CHILDHANDLERBASE_H_
#define FSFW_DEVICEHANDLER_CHILDHANDLERBASE_H_

#include "ChildHandlerFDIR.h"
#include "DeviceHandlerBase.h"

class ChildHandlerBase : public DeviceHandlerBase {
 public:
  ChildHandlerBase(object_id_t setObjectId, object_id_t deviceCommunication, CookieIF* cookie,
                   object_id_t hkDestination, uint32_t thermalStatePoolId,
                   uint32_t thermalRequestPoolId, object_id_t parent = objects::NO_OBJECT,
                   FailureIsolationBase* customFdir = nullptr, size_t cmdQueueSize = 20);

  virtual ~ChildHandlerBase();

  virtual ReturnValue_t initialize();

 protected:
  const uint32_t parentId;
  ChildHandlerFDIR childHandlerFdir;
};

#endif /* FSFW_DEVICEHANDLER_CHILDHANDLERBASE_H_ */
