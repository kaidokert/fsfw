#ifndef FSFW_TMTCPACKET_TMPACKETCREATOR_H
#define FSFW_TMTCPACKET_TMPACKETCREATOR_H

#include "fsfw/tmtcpacket/pus/PusIF.h"

struct PusTmParams {
  uint8_t pusVersion;
  uint8_t scTimeRefStatus;
  uint8_t service;
  uint8_t subservice;
  uint16_t messageTypeCounter;
  uint16_t destId;
  uint8_t* timestamp;
  size_t timestampLen;
};
class PusTmCreator : public PusIF {
 public:
  ~PusTmCreator() override = default;
  explicit PusTmCreator(PusTmParams params);

  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;

 private:
  PusTmParams params;
};
#endif  // FSFW_TMTCPACKET_TMPACKETCREATOR_H
