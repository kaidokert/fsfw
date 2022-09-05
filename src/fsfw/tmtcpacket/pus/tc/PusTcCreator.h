#ifndef FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
#define FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H

#include "fsfw/serialize/SerialBufferAdapter.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketIF.h"
#include "fsfw/tmtcpacket/pus/CustomUserDataIF.h"
#include "fsfw/tmtcpacket/pus/defs.h"
#include "fsfw/tmtcpacket/pus/tc/PusTcIF.h"

struct PusTcParams {
  PusTcParams(uint8_t service_, uint8_t subservice_) : service(service_), subservice(subservice_) {}

  void setRawAppData(const uint8_t *data, size_t len) {
    bufAdapter.setConstBuffer(data, len);
    appData = &bufAdapter;
  }

  void setSerializableAppData(const SerializeIF &serializable) { appData = &serializable; }

  uint8_t service;
  uint8_t subservice;
  uint8_t ackFlags = ecss::ACK_ALL;
  uint16_t sourceId = 0;
  SerialBufferAdapter<uint8_t> bufAdapter;
  const SerializeIF *appData = nullptr;
  uint8_t pusVersion = ecss::PusVersion::PUS_C;
};

/**
 * This class provides a high-level interface to create PUS TC packets and then @serialize
 * them into a raw byte format. It implements @SerializeIF for that purpose.
 * A custom time stamper can be set, with the implementation of @TimeStamperIF as the only
 * requirement.
 */
class PusTcCreator : public PusTcIF, public SerializeIF, public CustomUserDataIF {
 public:
  PusTcCreator(SpacePacketParams initSpParams, PusTcParams initPusParams);

  /**
   * If the parameter structure is changed in a way which changes the resulting serialized packet
   * size, this function should be called to set the data length field in the space packet
   * header. This fields is the primary source of information for length information.
   *
   * The only case for a telecommand where this size changes would be if user data is set.
   */
  void updateSpLengthField();
  PusTcParams &getPusParams();
  SpacePacketParams &getSpParams();

  [[nodiscard]] size_t getSerializedSize() const override;

  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getAcknowledgeFlags() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  [[nodiscard]] uint16_t getSourceId() const override;
  ReturnValue_t setRawUserData(const uint8_t *data, size_t len) override;
  ReturnValue_t setSerializableUserData(const SerializeIF &serializable) override;

  // Load all big endian helpers into the class namespace
  using SerializeIF::serializeBe;

 private:
  // Forbidden to use, enforce big endian serialization
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;
  // Forbidden to use
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  void setup();
  SpacePacketCreator spCreator;
  PusTcParams pusParams;
};

#endif  // FSFW_TMTCPACKET_TCPACKETDESERIALIZER_H
