#ifndef FSFW_DATALINKLAYER_MAPPACKETEXTRACTION_H_
#define FSFW_DATALINKLAYER_MAPPACKETEXTRACTION_H_

#include "MapPacketExtractionIF.h"
#include "dllConf.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/returnvalues/returnvalue.h"

class StorageManagerIF;

/**
 * Implementation of a MAP Packet Extraction class.
 * The class implements the full MAP Packet Extraction functionality as described in the CCSDS
 * TC Space Data Link Protocol. It internally stores incomplete segmented packets until they are
 * fully received. All found packets are forwarded to a single distribution entity.
 * @author B. Baetz
 */
class MapPacketExtraction : public MapPacketExtractionIF {
 private:
  static const uint32_t MAX_PACKET_SIZE = 4096;
  uint8_t lastSegmentationFlag;           //!< The segmentation flag of the last received frame.
  uint8_t mapId;                          //!< MAP ID of this MAP Channel.
  uint32_t packetLength = 0;              //!< Complete length of the current Space Packet.
  uint8_t* bufferPosition;                //!< Position to write to in the internal Packet buffer.
  uint8_t packetBuffer[MAX_PACKET_SIZE];  //!< The internal Space Packet Buffer.
  object_id_t packetDestination;
  //!< Pointer to the store where full TC packets are stored.
  StorageManagerIF* packetStore = nullptr;
  MessageQueueId_t tcQueueId;  //!< QueueId to send found packets to the distributor.
  /**
   * Debug method to print the packet Buffer's content.
   */
  void printPacketBuffer();
  /**
   * Method that is called if the segmentation flag is @c NO_SEGMENTATION.
   * The method extracts one or more packets within the frame and forwards them to the OBSW.
   * @param frame	The TC Transfer Frame to work on.
   * @return	@c returnvalue::OK if all Packets were extracted. If something is entirely wrong,
   * 	 @c DATA_CORRUPTED is returned, if some bytes are left over @c RESIDUAL_DATA.
   */
  ReturnValue_t unpackBlockingPackets(TcTransferFrame* frame);
  /**
   * Helper method to forward a complete packet to the OBSW.
   * @param data	Pointer to the data, either directly from the frame or from the
   * packetBuffer.
   * @param size	Complete total size of the packet.
   * @return	Return Code of the Packet Store or the Message Queue.
   */
  ReturnValue_t sendCompletePacket(uint8_t* data, uint32_t size);
  /**
   * Helper method to reset the internal buffer.
   */
  void clearBuffers();

 public:
  /**
   * Default constructor.
   * Members are set to default values.
   * @param setMapId	The MAP ID of the instance.
   */
  MapPacketExtraction(uint8_t setMapId, object_id_t setPacketDestination);
  ReturnValue_t extractPackets(TcTransferFrame* frame);
  /**
   * The #packetStore and the default destination of #tcQueue are initialized here.
   * @return	@c returnvalue::OK on success, @c returnvalue::FAILED otherwise.
   */
  ReturnValue_t initialize();
  /**
   * Getter.
   * @return The MAP ID of this instance.
   */
  uint8_t getMapId() const;
};

#endif /* FSFW_DATALINKLAYER_MAPPACKETEXTRACTION_H_ */
