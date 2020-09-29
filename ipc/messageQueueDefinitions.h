#ifndef FSFW_IPC_MESSAGEQUEUEDEFINITIONS_H_
#define FSFW_IPC_MESSAGEQUEUEDEFINITIONS_H_

#include <cstdint>

/*
 * TODO: Actually, the definition of this ID to be a uint32_t is not  ideal and
 * breaks layering. However, it is difficult to keep layering, as the ID is
 * stored in many places and sent around in MessageQueueMessage.
 * Ideally, one would use the (current) object_id_t only, however, doing a
 * lookup of queueIDs for every call does not sound ideal.
 * In a first step, I'll circumvent the issue by not touching it,
 * maybe in a second step. This also influences Interface design
 * (getCommandQueue) and some other issues..
 */
using MessageQueueId_t = uint32_t;

#endif /* FSFW_IPC_MESSAGEQUEUEDEFINITIONS_H_ */
