#ifndef FSFW_TMTCSERVICES_TMSTOREFRONTENDIF_H_
#define FSFW_TMTCSERVICES_TMSTOREFRONTENDIF_H_

#include "TmStorePackets.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "tmStorageConf.h"

class TmPacketMinimal;
class SpacePacketBase;
class TmStoreBackendIF;

class TmStoreFrontendIF {
 public:
  virtual TmStoreBackendIF* getBackend() const = 0;

  /**
   * What do I need to implement here?
   * This is propably used by PUS Service 15 so we should propably check for messages..
   * Provide base implementation?
   * @param opCode
   * @return
   */
  virtual ReturnValue_t performOperation(uint8_t opCode) = 0;
  /**
   * Callback from the back-end to indicate a certain packet was received.
   * front-end takes care of discarding/downloading the packet.
   * @param packet	Pointer to the newly received Space Packet.
   * @param address	Start address of the packet found
   * @param isLastPacket	Indicates if no more packets can be fetched.
   * @return	If more packets shall be fetched, returnvalue::OK must be returned.
   * 			Any other code stops fetching packets.
   */
  virtual ReturnValue_t packetRetrieved(TmPacketMinimal* packet, uint32_t address) = 0;
  virtual void noMorePacketsInStore() = 0;
  virtual void handleRetrievalFailed(ReturnValue_t errorCode, uint32_t parameter1 = 0,
                                     uint32_t parameter2 = 0) = 0;
  /**
   * To get the queue where commands shall be sent.
   * @return	Id of command queue.
   */
  virtual MessageQueueId_t getCommandQueue() const = 0;
  virtual ReturnValue_t fetchPackets(ApidSsc start, ApidSsc end) = 0;
  virtual ReturnValue_t deletePackets(ApidSsc upTo) = 0;
  virtual ReturnValue_t checkPacket(SpacePacketBase* tmPacket) = 0;
  virtual bool isEnabled() const = 0;
  virtual void setEnabled(bool enabled) = 0;
  virtual void resetDownlinkedPacketCount() = 0;
  virtual ReturnValue_t setDumpTarget(object_id_t dumpTarget) = 0;
  static const uint8_t INTERFACE_ID = CLASS_ID::TM_STORE_FRONTEND_IF;
  static const ReturnValue_t BUSY = MAKE_RETURN_CODE(1);
  static const ReturnValue_t LAST_PACKET_FOUND = MAKE_RETURN_CODE(2);
  static const ReturnValue_t STOP_FETCH = MAKE_RETURN_CODE(3);
  static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(4);
  static const ReturnValue_t TM_CHANNEL_FULL = MAKE_RETURN_CODE(5);
  static const ReturnValue_t NOT_STORED = MAKE_RETURN_CODE(6);
  static const ReturnValue_t ALL_DELETED = MAKE_RETURN_CODE(7);
  static const ReturnValue_t INVALID_DATA = MAKE_RETURN_CODE(8);
  static const ReturnValue_t NOT_READY = MAKE_RETURN_CODE(9);
  virtual ~TmStoreFrontendIF() {}
};

#endif /* FSFW_TMTCSERVICES_TMSTOREFRONTENDIF_H_ */
