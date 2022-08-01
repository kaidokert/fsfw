#ifndef FRAMEWORK_TMTCSERVICES_ACCEPTSTELECOMMANDSIF_H_
#define FRAMEWORK_TMTCSERVICES_ACCEPTSTELECOMMANDSIF_H_

#include "fsfw/ipc/MessageQueueSenderIF.h"

/**
 * @brief 	This interface is implemented by classes that are sinks for
 * 			Telecommands.
 * @details	Any service receiving telecommands shall implement this interface
 * 			and thus make the service id and the receiving message queue public.
 */
class AcceptsTelecommandsIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::ACCEPTS_TELECOMMANDS_IF;
  static const ReturnValue_t ACTIVITY_STARTED = MAKE_RETURN_CODE(1);
  static const ReturnValue_t INVALID_SUBSERVICE = MAKE_RETURN_CODE(2);
  static const ReturnValue_t ILLEGAL_APPLICATION_DATA = MAKE_RETURN_CODE(3);
  static const ReturnValue_t SEND_TM_FAILED = MAKE_RETURN_CODE(4);
  static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(5);
  /**
   * @brief The virtual destructor as it is mandatory for C++ interfaces.
   */
  virtual ~AcceptsTelecommandsIF() = default;
  [[nodiscard]] virtual const char* getName() const = 0;

  /**
   * @brief	Getter for a generic identifier ID.
   * @details	Any receiving service (at least any PUS service) shall have an identifier. For
   *    example, this could be the APID for a receiver expecting generic PUS packets, or a PUS
   *    service for a component expecting specific PUS service packets.
   * @return	The identifier.
   */
  [[nodiscard]] virtual uint32_t getIdentifier() const = 0;
  /**
   * @brief	This method returns the message queue id of the telecommand
   * 			receiving message queue.
   * @return	The telecommand reception message queue id.
   */
  [[nodiscard]] virtual MessageQueueId_t getRequestQueue() const = 0;
};

#endif /* FRAMEWORK_TMTCSERVICES_ACCEPTSTELECOMMANDSIF_H_ */
