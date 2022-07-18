#ifndef FSFW_TMTCPACKET_SPACEPACKETBASE_H_
#define FSFW_TMTCPACKET_SPACEPACKETBASE_H_

#include <fsfw/tmtcpacket/RedirectableDataPointerIF.h>

#include <cstddef>

#include "SpacePacketIF.h"
#include "ccsds_header.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

/**
 * @defgroup tmtcpackets Space Packets
 * This is the group, where all classes associated with Telecommand and
 * Telemetry packets belong to.
 * The class hierarchy resembles the dependency between the different standards
 * applied, namely the CCSDS Space Packet standard and the ECCSS Packet
 * Utilization Standard. Most field and structure names are taken from these
 * standards.
 */

/**
 * This struct defines the data structure of a Space Packet when accessed
 * via a pointer.
 * @ingroup tmtcpackets
 */
struct SpacePacketPointer {
  CCSDSPrimaryHeader header;
  uint8_t packet_data;
};

/**
 * This class is the basic data handler for any CCSDS Space Packet
 * compatible Telecommand and Telemetry packet.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * Remark: All bit numbers in this documentation are counted from
 * the most significant bit (from left).
 * @ingroup tmtcpackets
 */
class SpacePacketReader : public SpacePacketIF, public RedirectableDataPointerIF {
 protected:
  /**
   * A pointer to a structure which defines the data structure of
   * the packet header.
   * To be hardware-safe, all elements are of byte size.
   */
  SpacePacketPointer* data;

 public:
  /**
   * This definition defines the CRC size in byte.
   */
  static const uint8_t CRC_SIZE = 2;
  /**
   * This is the minimum size of a SpacePacket.
   */
  static const uint16_t MINIMUM_SIZE = sizeof(CCSDSPrimaryHeader) + CRC_SIZE;
  SpacePacketReader(uint16_t apid, ccsds::PacketType packetType, bool secHeader, uint16_t seqCount,
                    uint16_t dataLen);
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed.
   * @param set_address	The position where the packet data lies.
   */
  explicit SpacePacketReader(const uint8_t* set_address);
  /**
   * No data is allocated, so the destructor is empty.
   */
  ~SpacePacketReader() override;

  ReturnValue_t initSpacePacketHeader(bool isTelecommand, bool hasSecondaryHeader, uint16_t apid,
                                      uint16_t sequenceCount = 0);

  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;

  /**
   * Sets the APID of a packet, which are the lowest 11 bit of the packet
   * id.
   * @param 	The APID to set. The highest five bits of the parameter are
   * 			ignored.
   */
  void setApid(uint16_t setAPID);

  /**
   * Sets the sequence flags of a packet, which are bit 17 and 18 in the space packet header.
   * @param   The sequence flags to set
   */
  void setSequenceFlags(uint8_t sequenceflags);

  /**
   * Sets the packet sequence count, which are the lowest 14 bit of the
   * packet sequence control field.
   * setCount is modulo-divided by \c LIMIT_SEQUENCE_COUNT to avoid overflows.
   * @param setCount	The value to set the count to.
   */
  void setPacketSequenceCount(uint16_t setCount);

  /**
   * Sets the packet data length, which is the fifth and sixth byte of the
   * CCSDS Primary Header.
   * @param setLength The value of the length to set. It must fit the true
   * 					CCSDS packet data length . The packet data length is
   * 					the size of every kind of data \b after the CCSDS
   * 					Primary Header \b -1.
   */
  void setPacketDataLength(uint16_t setLength);

  // Helper methods:
  /**
   * This method returns a raw uint8_t pointer to the packet.
   * @return	A \c uint8_t pointer to the first byte of the CCSDS primary header.
   */
  virtual uint8_t* getWholeData();

  uint8_t* getPacketData();

  /**
   * With this method, the packet data pointer can be redirected to another
   * location.
   * @param p_Data A pointer to another raw Space Packet.
   */
  ReturnValue_t setData(uint8_t* p_Data, size_t maxSize, void* args) override;
  /**
   * This method returns the full raw packet size.
   * @return	The full size of the packet in bytes.
   */
  size_t getFullSize();
};

#endif /* FSFW_TMTCPACKET_SPACEPACKETBASE_H_ */
