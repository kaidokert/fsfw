#ifndef FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_
#define FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_

#include "fsfw/ipc/MessageQueueMessageIF.h"

class AcceptsHkPacketsIF {
 public:
  virtual ~AcceptsHkPacketsIF() = default;
  [[nodiscard]] virtual MessageQueueId_t getHkQueue() const = 0;
};

#endif /* FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_ */
