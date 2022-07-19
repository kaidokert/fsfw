#ifndef FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_
#define FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_

#include "PusTmIF.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/tmtcpacket/SpacePacketReader.h"

struct timeval;
class PacketTimestampInterpreterIF;
/**
 * This is a minimal version of a PUS TmPacket without any variable field, or,
 * in other words with Service Type, Subtype and subcounter only.
 * This is required for handling TM packets with different APIDs with different
 * secondary headers.
 */
class PusTmMinimal : public PusTmIF, public RedirectableDataPointerIF {
 public:
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
  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  const uint8_t* getUserData(size_t& appDataLen) const override;
  [[nodiscard]] uint16_t getUserDataSize() const override;
  uint8_t getScTimeRefStatus() override;
  uint16_t getMessageTypeCounter() override;
  uint16_t getDestId() override;
  const uint8_t* getTimestamp(size_t& timeStampLen) override;
  size_t getTimestampLen() override;

  // NOTE: Only PUS C compatible!
  struct PusTmMinimalSecHeader {
    uint8_t versionAndScTimeRefStatus;
    uint8_t service;
    uint8_t subservice;
    uint16_t messageTypeCounter;
  };

  /**
   * This struct defines the data structure of a PUS Telecommand Packet when
   * accessed via a pointer.
   * @ingroup tmtcpackets
   */
  struct TmPacketMinimalPointer {
    CCSDSPrimaryHeader primary;
    PusTmMinimalSecHeader secHeader;
    uint8_t rest;
  };
  // Must include a checksum and is therefore at least one larger than the above struct.
  static const uint16_t MINIMUM_SIZE = sizeof(TmPacketMinimalPointer) + 1;

 protected:
  /**
   * A pointer to a structure which defines the data structure of
   * the packet's data.
   *
   * To be hardware-safe, all elements are of byte size.
   */
  TmPacketMinimalPointer* tmData;

  static PacketTimestampInterpreterIF* timestampInterpreter;
};

#endif /* FRAMEWORK_TMTCPACKET_PUS_TMPACKETMINIMAL_H_ */
