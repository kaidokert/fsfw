#ifndef FSFW_TMTCPACKET_TMPACKETCREATOR_H
#define FSFW_TMTCPACKET_TMPACKETCREATOR_H

#include "PusTmIF.h"
#include "fsfw/serialize/SerialBufferAdapter.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/pus/CustomUserDataIF.h"

struct PusTmSecHeader {
  PusTmSecHeader() = default;
  PusTmSecHeader(uint8_t service, uint8_t subservice, TimeWriterIF* timeStamper)
      : service(service), subservice(subservice), timeStamper(timeStamper) {}

  uint8_t service = 0;
  uint8_t subservice = 0;
  TimeWriterIF* timeStamper = nullptr;
  uint8_t pusVersion = ecss::PusVersion::PUS_C;
  uint8_t scTimeRefStatus = 0;
  uint16_t messageTypeCounter = 0;
  uint16_t destId = 0;
};

struct PusTmParams {
  PusTmParams() = default;
  explicit PusTmParams(PusTmSecHeader secHeader) : secHeader(secHeader){};
  PusTmParams(PusTmSecHeader secHeader, const SerializeIF& data)
      : secHeader(secHeader), sourceData(&data) {}
  PusTmParams(PusTmSecHeader secHeader, const uint8_t* data, size_t dataLen)
      : secHeader(secHeader), adapter(data, dataLen), sourceData(&adapter) {}
  PusTmParams(uint8_t service, uint8_t subservice, TimeWriterIF* timeStamper)
      : secHeader(service, subservice, timeStamper) {}

  PusTmParams(uint8_t service, uint8_t subservice, TimeWriterIF* timeStamper,
              const SerializeIF& data_)
      : PusTmParams(service, subservice, timeStamper) {
    sourceData = &data_;
  }

  PusTmParams(uint8_t service, uint8_t subservice, TimeWriterIF* timeStamper, const uint8_t* data,
              size_t dataLen)
      : secHeader(service, subservice, timeStamper), adapter(data, dataLen), sourceData(&adapter) {}
  PusTmSecHeader secHeader;
  SerialBufferAdapter<uint8_t> adapter;
  const SerializeIF* sourceData = nullptr;
};

class TimeWriterIF;

/**
 * This class provides a high-level interface to create PUS TM packets and then @serialize
 * them into a raw byte format. It implements @SerializeIF for that purpose.
 * A custom time stamper can be set, with the implementation of @TimeStamperIF as the only
 * requirement.
 */
class PusTmCreator : public SerializeIF, public PusTmIF, public CustomUserDataIF {
 public:
  /**
   * Empty creator with all-default parameters. Please note that serializing this will
   * generate an invalid PUS packet with no timestamp.
   */
  PusTmCreator();
  PusTmCreator(SpacePacketParams initSpParams, PusTmParams initPusParams);
  ~PusTmCreator() override = default;

  void setTimeStamper(TimeWriterIF& timeStamper);
  /**
   * This function disables the CRC16 calculation on serialization. This is useful to avoid
   * duplicate calculation if some lower level component needs to update fields like the sequence
   * count, which would require a checksum update.
   */
  void disableCrcCalculation();
  void enableCrcCalculation();
  [[nodiscard]] bool crcCalculationEnabled() const;

  SpacePacketParams& getSpParams();
  void setApid(uint16_t apid);
  void setDestId(uint16_t destId);
  void setService(uint8_t service);
  void setSubservice(uint8_t subservice);
  void setMessageTypeCounter(uint16_t messageTypeCounter);
  PusTmParams& getParams();
  void updateSpLengthField();
  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  uint8_t getScTimeRefStatus() override;
  uint16_t getMessageTypeCounter() override;
  uint16_t getDestId() override;
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;
  [[nodiscard]] size_t getSerializedSize() const override;
  [[nodiscard]] TimeWriterIF* getTimestamper() const;
  ReturnValue_t setRawUserData(const uint8_t* data, size_t len) override;
  ReturnValue_t setSerializableUserData(const SerializeIF& serializable) override;

  // Load all big endian (network endian) helpers into scope
  using SerializeIF::serializeBe;

 private:
  // Forbidden to use
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;
  void setup();
  PusTmParams pusParams{};
  bool calculateCrcOnSerialization = true;
  SpacePacketCreator spCreator;
};
#endif  // FSFW_TMTCPACKET_TMPACKETCREATOR_H
