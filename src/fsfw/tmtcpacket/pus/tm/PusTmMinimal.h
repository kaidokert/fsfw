#ifndef FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_
#define FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_

#include "PusTmIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"
#include "fsfw/tmtcpacket/pus/RawUserDataReaderIF.h"

struct timeval;

class PacketTimestampInterpreterIF;

namespace mintm {

// NOTE: Only PUS C compatible!
struct PusTmMinimalSecHeaderPacked {
  uint8_t versionAndScTimeRefStatus;
  uint8_t service;
  uint8_t subservice;
  uint8_t messageTypeH;
  uint8_t messageTypeL;
};

/**
 * This struct defines the data structure of a PUS Telecommand Packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct MinimalPusTm {
  ccsds::PrimaryHeader primary;
  PusTmMinimalSecHeaderPacked secHeader;
  uint8_t rest;
};

// Must include a checksum and is therefore at least one larger than the above struct.
static const uint16_t MINIMUM_SIZE = sizeof(MinimalPusTm) + 1;

}  // namespace mintm

/**
 * This is a minimal version of a PUS TmPacket without any variable field, or,
 * in other words with Service Type, Subtype and subcounter only.
 * This is required for handling TM packets with different APIDs with different
 * secondary headers.
 */
class PusTmMinimal : public PusTmIF, public RawUserDataReaderIF, public RedirectableDataPointerIF {
 public:
  explicit PusTmMinimal(mintm::MinimalPusTm* data);
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed and also
   * forwards the data pointer to the parent SpacePacketBase class.
   * @param set_address	The position where the packet data lies.
   */
  explicit PusTmMinimal(uint8_t* data);
  /**
   * This is the empty default destructor.
   */
  ~PusTmMinimal() override;

  void setApid(uint16_t apid);

  ReturnValue_t getPacketTime(timeval* timestamp);

  static void setInterpretTimestampObject(PacketTimestampInterpreterIF* interpreter);
  ReturnValue_t setData(uint8_t* dataPtr, size_t size, void* args) override;
  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  uint8_t getScTimeRefStatus() override;
  uint16_t getMessageTypeCounter() override;
  uint16_t getDestId() override;
  const uint8_t* getUserData() const override;
  size_t getUserDataLen() const override;

 protected:
  /**
   * A pointer to a structure which defines the data structure of
   * the packet's data.
   *
   * To be hardware-safe, all elements are of byte size.
   */
  size_t userDataLen = 0;
  mintm::MinimalPusTm* tmData;

  static PacketTimestampInterpreterIF* timestampInterpreter;
};

#endif /* FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_ */
