#ifndef FSFW_TMTCSERVICES_TMTCMESSAGE_H_
#define FSFW_TMTCSERVICES_TMTCMESSAGE_H_

#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
/**
 * @brief	This message class is used to pass Telecommand and Telemetry
 * 			packets between tasks.
 * @details	Within such a packet, nothing is transported but the identifier of
 * 			a packet stored in one of the IPC stores (typically a special TM and
 * 			a special TC store). This makes passing commands very simple and
 * 			efficient.
 * @ingroup message_queue
 */
class TmTcMessage : public MessageQueueMessage {
 protected:
  /**
   * @brief	This call always returns the same fixed size of the message.
   * @return	Returns HEADER_SIZE + @c sizeof(store_address_t).
   */
  [[nodiscard]] size_t getMinimumMessageSize() const override;

 public:
  /**
   * @brief	In the default constructor, only the message_size is set.
   */
  TmTcMessage();
  /**
   * @brief	With this constructor, the passed packet id is directly put
   * 			into the message.
   * @param packet_id	The packet id to put into the message.
   */
  explicit TmTcMessage(store_address_t packetId);
  /**
   * @brief	The class's destructor is empty.
   */
  ~TmTcMessage() override;
  /**
   * @brief	This getter returns the packet id in the correct format.
   * @return	Returns the packet id.
   */
  store_address_t getStorageId();
  /**
   * @brief	In some cases it might be useful to have a setter for packet id
   * 			as well.
   * @param packetId	The packet id to put into the message.
   */
  void setStorageId(store_address_t packetId);
};

#endif /* FSFW_TMTCSERVICES_TMTCMESSAGE_H_ */
