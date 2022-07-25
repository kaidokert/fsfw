#ifndef FSFW_TIMEMANAGER_TIMEMESSAGE_H_
#define FSFW_TIMEMANAGER_TIMEMESSAGE_H_

#include <cstring>

#include "Clock.h"
#include "fsfw/ipc/MessageQueueMessage.h"

class TimeMessage : public MessageQueueMessage {
 protected:
  /**
   * @brief	This call always returns the same fixed size of the message.
   * @return	Returns HEADER_SIZE + \c sizeof(timeval) + sizeof(uint32_t).
   */
  size_t getMinimumMessageSize() const override;

 public:
  /**
   * @ brief the size of a TimeMessage
   */
  static const uint32_t MAX_SIZE = HEADER_SIZE + sizeof(timeval) + sizeof(uint32_t);

  /**
   * @brief	In the default constructor, only the message_size is set.
   */
  TimeMessage();
  /**
   * @brief	With this constructor, the passed time information is directly put
   * 			into the message.
   * @param 	setTime The time information to put into the message.
   * @param 	counterValue The counterValue to put into the message (GPS PPS).
   */
  TimeMessage(timeval setTime, uint32_t counterValue = 0);
  /**
   * @brief	The class's destructor is empty.
   */
  ~TimeMessage();
  /**
   * @brief	This getter returns the time information in timeval format.
   * @return	Returns the time stored in this packet.
   */
  timeval getTime();
  /**
   * @brief 	This getter returns the CounterValue in uint32_t format.
   * @return 	Returns the CounterValue stored in this packet.
   */
  uint32_t getCounterValue();
};

#endif /* FSFW_TIMEMANAGER_TIMEMESSAGE_H_ */
