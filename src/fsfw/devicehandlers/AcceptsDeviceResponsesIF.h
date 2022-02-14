#ifndef FSFW_DEVICEHANDLERS_ACCEPTSDEVICERESPONSESIF_H_
#define FSFW_DEVICEHANDLERS_ACCEPTSDEVICERESPONSESIF_H_

#include "../ipc/MessageQueueSenderIF.h"

/**
 * This interface is used by the device handler to send a device response
 * to the queue ID, which is returned in the implemented abstract method.
 */
class AcceptsDeviceResponsesIF {
 public:
  /**
   * Default empty virtual destructor.
   */
  virtual ~AcceptsDeviceResponsesIF() {}
  virtual MessageQueueId_t getDeviceQueue() = 0;
};

#endif /* FSFW_DEVICEHANDLERS_ACCEPTSDEVICERESPONSESIF_H_ */
