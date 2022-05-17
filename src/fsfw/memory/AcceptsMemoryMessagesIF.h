#ifndef FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_
#define FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "HasMemoryIF.h"
#include "MemoryMessage.h"

class AcceptsMemoryMessagesIF : public HasMemoryIF {
 public:
  virtual MessageQueueId_t getCommandQueue() const = 0;
};

#endif /* FSFW_MEMORY_ACCEPTSMEMORYMESSAGESIF_H_ */
