#ifndef FSFW_TMTCSERVICES_TCDISTRIBUTOR_H_
#define FSFW_TMTCSERVICES_TCDISTRIBUTOR_H_

#include <map>

#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

/**
 * @defgroup tc_distribution Telecommand Distribution
 * All classes associated with Routing and Distribution of Telecommands
 * belong to this group.
 */

/**
 * This is the base class to implement distributors for Space Packets.
 * Typically, the distribution is required to forward Telecommand packets
 * over the satellite applications and services. The class receives
 * Space Packets over a message queue and holds a map that links other
 * message queue ids to some identifier. The process of unpacking the
 * destination information from the packet is handled by the child class
 * implementations.
 * @ingroup tc_distribution
 */
class TcDistributor : public SystemObject, public ExecutableObjectIF {
 public:
  using TcMessageQueueMap = std::map<uint32_t, MessageQueueId_t>;
  using TcMqMapIter = std::map<uint32_t, MessageQueueId_t>::iterator;

  static constexpr uint8_t INTERFACE_ID = CLASS_ID::PACKET_DISTRIBUTION;
  static constexpr ReturnValue_t PACKET_LOST = MAKE_RETURN_CODE(1);
  static constexpr ReturnValue_t DESTINATION_NOT_FOUND = MAKE_RETURN_CODE(2);
  static constexpr ReturnValue_t SERVICE_ID_ALREADY_EXISTS = MAKE_RETURN_CODE(3);
  /**
   * Within the default constructor, the SystemObject id is set and the
   * message queue is initialized.
   * Filling the map is under control of the child classes.
   * @param set_object_id	This id is assigned to the distributor
   * 		implementation.
   */
  TcDistributor(object_id_t objectId);
  /**
   * The destructor is empty, the message queues are not in the vicinity of
   * this class.
   */
  virtual ~TcDistributor();
  /**
   * The method is called cyclically and fetches new incoming packets from
   * the message queue.
   * In case a new packet is found, it calls the handlePacket method to deal
   * with distribution.
   * @return The error code of the message queue call.
   */
  ReturnValue_t performOperation(uint8_t opCode);
  /**
   * A simple debug print, that prints all distribution information stored in
   * queueMap.
   */
  void print();

 protected:
  /**
   * This is the receiving queue for incoming Telecommands.
   * The child classes must make its queue id public.
   */
  MessageQueueIF* tcQueue = nullptr;
  /**
   * The last received incoming packet information is stored in this
   * member.
   * As different child classes unpack the incoming packet differently
   * (i.e. as a CCSDS Space Packet or as a PUS Telecommand Packet), it
   * is not tried to unpack the packet information within this class.
   */
  TmTcMessage currentMessage;
  /**
   * The map that links certain packet information to a destination.
   * The packet information may be the APID of the packet or the service
   * identifier. Filling of the map is under control of the different child
   * classes.
   */
  TcMessageQueueMap queueMap;
  /**
   * This method shall unpack the routing information from the incoming
   * packet and select the map entry which represents the packet's target.
   * @return	An iterator to the map element to forward to or queuMap.end().
   */
  virtual TcMqMapIter selectDestination() = 0;
  /**
   * The handlePacket method calls the child class's selectDestination method
   * and forwards the packet to its destination, if found.
   * @return The message queue return value or @c returnvalue::FAILED, in case no
   * 		destination was found.
   */
  ReturnValue_t handlePacket();
  /**
   * This method gives the child class a chance to perform some kind of
   * operation after the parent tried to forward the message.
   * A typically application would be sending success/failure messages.
   * The default implementation just returns @c returnvalue::OK.
   * @param queueStatus	The status of the message queue after an attempt
   *                      to send the TC.
   * @return	- @c returnvalue::OK on success
   * 		- @c returnvalue::FAILED on failure
   */
  virtual ReturnValue_t callbackAfterSending(ReturnValue_t queueStatus);

 private:
  /**
   * This constant sets the maximum number of packets distributed per call.
   */
  static constexpr uint8_t DISTRIBUTER_MAX_PACKETS = 128;
};

#endif /* FSFW_TMTCSERVICES_TCDISTRIBUTOR_H_ */
