#ifndef FSFW_TMTCPACKET_SPACEPACKETBASE_H_
#define FSFW_TMTCPACKET_SPACEPACKETBASE_H_

#include <cstddef>

#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tmtcpacket/ReadablePacketIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/ccsds/defs.h"

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
 * data must be set on instantiation or with the @setData or @setReadOnlyData call.
 * The @isNull and @checkSize methods can be used to check the validity of the data pointed to.
 *
 * This is a zero-copy reader class. It does not contain the packet data itself but a pointer to
 * the data. Calling any accessor methods without pointing the object to valid data  first will
 * cause undefined behaviour.
 * @ingroup tmtcpackets
 */
class SpacePacketReader : public SpacePacketIF,
                          public ReadablePacketIF,
                          public RedirectableDataPointerIF {
 public:
  /**
   * Initialize an empty space packet reader which points to no data
   */
  SpacePacketReader() = default;
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed.
   * @param set_address	The position where the packet data lies.
   */
  explicit SpacePacketReader(const uint8_t* setAddress, size_t maxSize);
  /**
   * No data is allocated, so the destructor is empty.
   */
  ~SpacePacketReader() override;

  /**
   * Check whether any data is set for the reader object
   * @return
   */
  [[nodiscard]] bool isNull() const;
  /**
   * Get size of the buffer. This is the size which is passed to the constructor or to the
   * @setData call. It is not the content of the CCSDS data length field and it is not necessarily
   * equal to the full packet length of the space packet.
   * @return
   */
  [[nodiscard]] size_t getBufSize() const;

  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;

  [[nodiscard]] const uint8_t* getFullData() const override;

  // Helper methods:
  [[nodiscard]] ReturnValue_t checkSize() const;

  const uint8_t* getPacketData();

  ReturnValue_t setReadOnlyData(const uint8_t* data, size_t maxSize);

 protected:
  /**
   * A pointer to a structure which defines the data structure of
   * the packet header.
   * To be hardware-safe, all elements are of byte size.
   */
  const ccsds::PrimaryHeader* spHeader{};
  const uint8_t* packetDataField{};
  size_t bufSize = 0;
  /**
   * With this method, the packet data pointer can be redirected to another
   * location.
   * @param data A pointer to another raw Space Packet.
   */
  ReturnValue_t setData(uint8_t* data, size_t maxSize, void* args) override;

  ReturnValue_t setInternalFields(const uint8_t* data, size_t maxSize);
};

#endif /* FSFW_TMTCPACKET_SPACEPACKETBASE_H_ */
