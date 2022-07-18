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
   * This is the default constructor.
   * It sets its internal data pointer to the address passed.
   * @param set_address	The position where the packet data lies.
   */
  explicit SpacePacketReader(const uint8_t* set_address);
  /**
   * No data is allocated, so the destructor is empty.
   */
  ~SpacePacketReader() override;

  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;

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
