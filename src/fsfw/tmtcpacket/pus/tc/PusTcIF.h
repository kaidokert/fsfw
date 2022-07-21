#ifndef FSFW_TMTCPACKET_PUSTCIF_H
#define FSFW_TMTCPACKET_PUSTCIF_H

#include <cstdint>

#include "fsfw/tmtcpacket/pus/PusIF.h"

namespace ecss {

enum AckField {
  //! No acknowledgements are expected.
  ACK_NONE = 0b0000,
  //! Acknowledgements on acceptance are expected.
  ACK_ACCEPTANCE = 0b0001,
  //! Acknowledgements on start are expected.
  ACK_START = 0b0010,
  //! Acknowledgements on step are expected.
  ACK_STEP = 0b0100,
  //! Acknowledgement on completion are expected.
  ACK_COMPLETION = 0b1000
};

static constexpr uint8_t ACK_ALL = ACK_ACCEPTANCE | ACK_START | ACK_STEP | ACK_COMPLETION;

/**
 * This struct defines a byte-wise structured PUS C data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, the Source Id field is present with one byte.
 * No spare byte support for now.
 * @ingroup tmtcpackets
 */
struct PusTcDataFieldHeader {
  // Version and ACK byte, Service Byte, Subservice Byte, 2 byte Source ID
  static constexpr size_t MIN_SIZE = 5;
  uint8_t pusVersion;
  uint8_t ackFlags;
  uint8_t serviceType;
  uint8_t serviceSubtype;
  uint16_t sourceId;
};

}  // namespace ecss

class PusTcIF : public PusIF {
 public:
  ~PusTcIF() override = default;
  static const size_t MIN_SIZE =
      (sizeof(ccsds::PrimaryHeader) + ecss::PusTcDataFieldHeader::MIN_SIZE + 2);

  /**
   * This is a getter for the packet's Ack field, which are the lowest four
   * bits of the first byte of the Data Field Header.
   *
   * It is packed in a uint8_t variable.
   * @return	The packet's PUS Ack field.
   */
  [[nodiscard]] virtual uint8_t getAcknowledgeFlags() const = 0;

  /**
   * The source ID can be used to have an additional identifier, e.g. for different ground
   * station.
   * @return
   */
  [[nodiscard]] virtual uint16_t getSourceId() const = 0;
};

#endif  // FSFW_TMTCPACKET_PUSTCIF_H
