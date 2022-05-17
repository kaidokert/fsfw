#ifndef FSFW_TMTCPACKET_PUS_TMPACKETSTOREDPUSC_H_
#define FSFW_TMTCPACKET_PUS_TMPACKETSTOREDPUSC_H_

#include "TmPacketPusC.h"
#include "TmPacketStoredBase.h"

/**
 *  This class generates a ECSS PUS C Telemetry packet within a given
 *  intermediate storage.
 *  As most packets are passed between tasks with the help of a storage
 *  anyway, it seems logical to create a Packet-In-Storage access class
 *  which saves the user almost all storage handling operation.
 *  Packets can both be newly created with the class and be "linked" to
 *  packets in a store with the help of a storeAddress.
 *  @ingroup tmtcpackets
 */
class TmPacketStoredPusC : public TmPacketStoredBase, public TmPacketPusC {
 public:
  /**
   * This is a default constructor which does not set the data pointer.
   * However, it does try to set the packet store.
   */
  TmPacketStoredPusC(store_address_t setAddress);
  /**
   * With this constructor, new space is allocated in the packet store and
   * a new PUS Telemetry Packet is created there.
   * Packet Application Data passed in data is copied into the packet.
   * The Application data is passed in two parts, first a header, then a
   * data field. This allows building a Telemetry Packet from two separate
   * data sources.
   * @param apid          Sets the packet's APID field.
   * @param service       Sets the packet's Service ID field.
   *      This specifies the source service.
   * @param subservice    Sets the packet's Service Subtype field.
   *      This specifies the source sub-service.
   * @param packet_counter    Sets the Packet counter field of this packet
   * @param data          The payload data to be copied to the
   *                      Application Data Field
   * @param size          The amount of data to be copied.
   * @param headerData    The header Data of the Application field,
   *                       will be copied in front of data
   * @param headerSize    The size of the headerDataF
   * @param destinationId Destination ID containing the application process ID as specified
   *                      by PUS C
   * @param timeRefField  4 bit time reference field as specified by PUS C
   */
  TmPacketStoredPusC(uint16_t apid, uint8_t service, uint8_t subservice, uint16_t packetCounter = 0,
                     const uint8_t* data = nullptr, uint32_t size = 0,
                     const uint8_t* headerData = nullptr, uint32_t headerSize = 0,
                     uint16_t destinationId = 0, uint8_t timeRefField = 0);
  /**
   * Another ctor to directly pass structured content and header data to the
   * packet to avoid additional buffers.
   */
  TmPacketStoredPusC(uint16_t apid, uint8_t service, uint8_t subservice, uint16_t packetCounter,
                     SerializeIF* content, SerializeIF* header = nullptr,
                     uint16_t destinationId = 0, uint8_t timeRefField = 0);

  uint8_t* getAllTmData() override;

 private:
  /**
   * Implementation required by base class
   * @param newPointer
   * @param maxSize
   * @param args
   * @return
   */
  ReturnValue_t setData(uint8_t* newPointer, size_t maxSize, void* args = nullptr) override;
};

#endif /* FSFW_TMTCPACKET_PUS_TMPACKETSTOREDPUSC_H_ */
