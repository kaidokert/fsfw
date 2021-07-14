#ifndef FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_
#define FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_

#include "../ipc/MessageQueueMessageIF.h"

class AcceptsHkPacketsIF {
public:
    virtual~ AcceptsHkPacketsIF() {};
    virtual MessageQueueId_t getHkQueue() const = 0;
};

#endif /* FRAMEWORK_HOUSEKEEPING_ACCEPTSHKPACKETSIF_H_ */
