#ifndef FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
#define FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H

#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/SpacePacketIF.h"
#include "fsfw/tmtcpacket/pus/definitions.h"
#include "fsfw/tmtcpacket/pus/tc/PusTcIF.h"

struct PusTcParams {
  PusTcParams(uint8_t service_, uint8_t subservice_) : service(service_), subservice(subservice_) {}

  uint8_t service;
  uint8_t subservice;
  uint8_t ackFlags = ecss::ACK_ALL;
  uint16_t sourceId = 0;
  uint8_t *appData = nullptr;
  size_t appDataLen = 0;
  uint8_t pusVersion = ecss::PusVersion::PUS_C;
};

class PusTcCreator : public PusTcIF, public SerializeIF {
 public:
  PusTcCreator(SpacePacketParams spParams, PusTcParams pusParams);

  void updateSpLengthField();
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;
  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getAcknowledgeFlags() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  [[nodiscard]] uint16_t getSourceId() const override;
  const uint8_t *getUserData(size_t &appDataLen) const override;
  [[nodiscard]] uint16_t getUserDataSize() const override;

 private:
  SpacePacketCreator spCreator;
  PusTcParams pusParams;
};

#endif  // FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
