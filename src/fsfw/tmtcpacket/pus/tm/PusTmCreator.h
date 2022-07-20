#ifndef FSFW_TMTCPACKET_TMPACKETCREATOR_H
#define FSFW_TMTCPACKET_TMPACKETCREATOR_H

#include "PusTmIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/pus/CreatorDataIF.h"

struct PusTmSecHeader {
  uint8_t pusVersion;
  uint8_t scTimeRefStatus;
  uint8_t service;
  uint8_t subservice;
  uint16_t messageTypeCounter;
  uint16_t destId;
  TimeStamperIF* timeStamper;
};

struct PusTmParams {
  PusTmSecHeader secHeader;
  ecss::DataWrapper dataWrapper;
};

class TimeStamperIF;

class PusTmCreator : public SerializeIF, public PusTmIF, public CreatorDataIF {
 public:
  PusTmCreator();

  explicit PusTmCreator(TimeStamperIF* timeStamper);
  PusTmCreator(SpacePacketParams initSpParams, PusTmParams initPusParams,
               TimeStamperIF* timeStamper);
  ~PusTmCreator() override = default;

  void setTimeStamper(TimeStamperIF* timeStamper);
  SpacePacketParams& getSpParams();
  void setApid(uint16_t apid);
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
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;
  TimeStamperIF* getTimestamper() override;

 private:
  ecss::DataWrapper& getDataWrapper() override;

 private:
  PusTmParams pusParams{};
  SpacePacketCreator spCreator;
};
#endif  // FSFW_TMTCPACKET_TMPACKETCREATOR_H
