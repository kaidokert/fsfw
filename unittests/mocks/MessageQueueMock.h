#ifndef FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_
#define FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_

#include <cstring>
#include <map>
#include <queue>

#include "CatchDefinitions.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueBase.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MessageQueueMessage.h"

struct SendInfo {
  explicit SendInfo(MessageQueueMessage& initMsg, unsigned int initCallCnt = 1)
      : callCount(initCallCnt) {
    msgs.push(initMsg);
  }
  unsigned int callCount = 0;
  std::queue<MessageQueueMessage> msgs;
};

class MessageQueueMock : public MessageQueueBase {
 public:
  void addReceivedMessage(MessageQueueMessageIF& msg);
  explicit MessageQueueMock(MessageQueueId_t queueId);

  //! Get next message which was sent to the default destination
  ReturnValue_t getNextSentMessage(MessageQueueMessageIF& message);
  //! Get message which was sent to a specific ID
  ReturnValue_t getNextSentMessage(MessageQueueId_t id, MessageQueueMessageIF& message);
  [[nodiscard]] bool wasMessageSent() const;
  void makeNextSendFail(ReturnValue_t retval);
  [[nodiscard]] size_t numberOfSentMessages() const;
  [[nodiscard]] size_t numberOfSentMessagesToDefault() const;
  [[nodiscard]] size_t numberOfSentMessagesToDest(MessageQueueId_t id) const;
  /**
   * Pop a message, clearing it in the process.
   * @return
   */
  ReturnValue_t clearLastReceivedMessage(bool clearCmdMsg = true);

  ReturnValue_t flush(uint32_t* count) override;
  ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                MessageQueueId_t sentFrom, bool ignoreFault = false) override;
  ReturnValue_t reply(MessageQueueMessageIF* message) override;

  ReturnValue_t clearLastSentMessage(MessageQueueId_t destId, bool clearCmdMsg = true);
  ReturnValue_t clearLastSentMessage(bool clearCmdMsg = true);
  void clearMessages(bool clearCmdMsg = true);

 private:
  using SendMap = std::map<MessageQueueId_t, SendInfo>;
  SendMap sendMap;
  std::queue<MessageQueueMessage> receivedMsgs;
  std::pair<bool, ReturnValue_t> nextSendFailsPair;

  void clearEmptyEntries();
  ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override;
  static ReturnValue_t clearLastSentMessage(SendMap::iterator& iter, bool clearCmdMsg = true);
  static void createMsgCopy(MessageQueueMessageIF& into, MessageQueueMessageIF& from);
};

#endif /* FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_ */
