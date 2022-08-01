#include "AcceptsTcMock.h"

AcceptsTcMock::AcceptsTcMock(const char* name, uint32_t id, MessageQueueId_t queueId)
    : name(name), id(id), queueId(queueId) {}

const char* AcceptsTcMock::getName() const { return name; }
uint32_t AcceptsTcMock::getIdentifier() const { return id; }
MessageQueueId_t AcceptsTcMock::getRequestQueue() const { return queueId; }
