/**
 * \file AcceptsThermalMessagesIF.h
 *
 * \date 16.02.2020
 */

#ifndef FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_
#define FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_
#include "../ipc/MessageQueueSenderIF.h"

class AcceptsThermalMessagesIF {
 public:
  /**
   * @brief	This is the empty virtual destructor as required for C++ interfaces.
   */
  virtual ~AcceptsThermalMessagesIF() {}

  virtual MessageQueueId_t getReceptionQueue() const = 0;
};

#endif /* FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_ */
