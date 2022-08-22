#ifndef UNITTEST_HOSTED_TESTACTIONHELPER_H_
#define UNITTEST_HOSTED_TESTACTIONHELPER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/ipc/MessageQueueIF.h>

#include <cstring>

#include "CatchDefinitions.h"

class ActionHelperOwnerMockBase : public HasActionsIF {
 public:
  bool getCommandQueueCalled = false;
  bool executeActionCalled = false;
  static const size_t MAX_SIZE = 3;
  uint8_t buffer[MAX_SIZE] = {0, 0, 0};
  size_t size = 0;

  MessageQueueId_t getCommandQueue() const override { return tconst::testQueueId; }

  ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy,
                              const uint8_t* data, size_t size) override {
    executeActionCalled = true;
    if (size > MAX_SIZE) {
      return 0xAFFE;
    }
    this->size = size;
    memcpy(buffer, data, size);
    return returnvalue::OK;
  }

  void clearBuffer() {
    this->size = 0;
    for (size_t i = 0; i < MAX_SIZE; i++) {
      buffer[i] = 0;
    }
  }

  void getBuffer(const uint8_t** ptr, size_t* size) {
    if (size != nullptr) {
      *size = this->size;
    }
    if (ptr != nullptr) {
      *ptr = buffer;
    }
  }
};

#endif /* UNITTEST_TESTFW_NEWTESTS_TESTACTIONHELPER_H_ */
