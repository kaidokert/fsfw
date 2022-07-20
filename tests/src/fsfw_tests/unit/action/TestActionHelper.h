#ifndef UNITTEST_HOSTED_TESTACTIONHELPER_H_
#define UNITTEST_HOSTED_TESTACTIONHELPER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/action/Parameter.h>
#include <fsfw/action/TemplateAction.h>
#include <fsfw/introspection/Enum.h>
#include <fsfw/ipc/MessageQueueIF.h>

#include <cstring>

#include "fsfw_tests/unit/CatchDefinitions.h"

class ActionHelperOwnerMockBase;

FSFW_ENUM(TestActions, ActionId_t, ((TEST_ACTION, "Test Action")))

class TestAction : public TemplateAction < ActionHelperOwnerMockBase, TestAction, TestActions> {
 public:
  TestAction(ActionHelperOwnerMockBase* owner) : TemplateAction(owner, TestActions::TEST_ACTION) {}
  Parameter<uint8_t> p1 = Parameter<uint8_t>::createParameter(this, "An uint8_t");
  Parameter<uint8_t> p2 = Parameter<uint8_t>::createParameter(this, "An uint8_t");
  Parameter<uint8_t> p3 = Parameter<uint8_t>::createParameter(this, "An uint8_t");
};

class ActionHelperOwnerMockBase : public HasActionsIF {
 public:
  bool getCommandQueueCalled = false;
  bool executeActionCalled = false;
  static const size_t MAX_SIZE = 3;
  uint8_t buffer[MAX_SIZE] = {0, 0, 0};
  size_t size = 0;

  ActionHelperOwnerMockBase(MessageQueueIF* useThisQueue) : actionHelper(this, useThisQueue) {}

  MessageQueueId_t getCommandQueue() const override { return tconst::testQueueId; }

  ActionHelper* getActionHelper() override { return &actionHelper; }

  ReturnValue_t executeAction(Action* action) override {
    executeActionCalled = true;
    if (size > MAX_SIZE) {
      return 0xAFFE;
    }
    this->size = size;
    return action->handle();
  }

  ReturnValue_t handleAction(TestAction *action){
    executeActionCalled = true;
    buffer[0] = action->p1;
    buffer[1] = action->p2;
    buffer[2] = action->p3;
    return HasReturnvaluesIF::RETURN_OK;
  }

  void clearBuffer() {
    this->size = 0;
    for (size_t i = 0; i < MAX_SIZE; i++) {
      buffer[i] = 0;
    }
  }

  void getBuffer(const uint8_t** ptr) {
    if (ptr != nullptr) {
      *ptr = buffer;
    }
  }

 private:
  ActionHelper actionHelper;
  TestAction testAction = TestAction(this);
};

#endif /* UNITTEST_TESTFW_NEWTESTS_TESTACTIONHELPER_H_ */
