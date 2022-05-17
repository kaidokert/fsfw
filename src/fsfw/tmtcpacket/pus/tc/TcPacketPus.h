#ifndef FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_

#include <cstdint>

#include "../definitions.h"
#include "TcPacketPusBase.h"
#include "fsfw/FSFW.h"
#include "fsfw/tmtcpacket/ccsds_header.h"

/**
 * This struct defines a byte-wise structured PUS TC A Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, the Source Id field is present with one byte.
 * @ingroup tmtcpackets
 */
struct PUSTcDataFieldHeader {
  uint8_t versionTypeAck;
  uint8_t serviceType;
  uint8_t serviceSubtype;
#if FSFW_USE_PUS_C_TELECOMMANDS == 1
  uint8_t sourceIdH;
  uint8_t sourceIdL;
#else
  uint8_t sourceId;
#endif
};

/**
 * This struct defines the data structure of a PUS Telecommand A packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct TcPacketPointer {
  CCSDSPrimaryHeader primary;
  PUSTcDataFieldHeader dataField;
  uint8_t appData;
};

class TcPacketPus : public TcPacketPusBase {
 public:
  static const uint16_t TC_PACKET_MIN_SIZE =
      (sizeof(CCSDSPrimaryHeader) + sizeof(PUSTcDataFieldHeader) + 2);

  /**
   * Initialize a PUS A telecommand packet which already exists. You can also
   * create an empty (invalid) object by passing nullptr as the data pointer
   * @param setData
   */
  TcPacketPus(const uint8_t* setData);

  /**
   * Initializes the Tc Packet header.
   * @param apid APID used.
   * @param sequenceCount Sequence Count in the primary header.
   * @param ack Which acknowledeges are expected from the receiver.
   * @param service   PUS Service
   * @param subservice PUS Subservice
   */
  void initializeTcPacket(uint16_t apid, uint16_t sequenceCount, uint8_t ack, uint8_t service,
                          uint8_t subservice, pus::PusVersion pusVersion, uint16_t sourceId = 0);
  /**
   * Set the application data field by copying the provided data to the application data field.
   * This function can also update the space packet length
   * field. To only update the SP length field with empty application data, simply pass 0 as
   * length of nullptr as data.
   * @param data
   * @param len
   * @param updateSpLenField
   */
  void setApplicationData(const uint8_t* data, size_t len,  bool updateSpLenField);

  // Base class overrides
  uint8_t getSecondaryHeaderFlag() const override;
  uint8_t getPusVersionNumber() const override;
  uint8_t getAcknowledgeFlags() const override;
  uint8_t getService() const override;
  uint8_t getSubService() const override;
  uint16_t getSourceId() const override;
  const uint8_t* getApplicationData() const override;
  uint16_t getApplicationDataSize() const override;
  uint16_t getErrorControl() const override;
  void setErrorControl() override;
  size_t calculateFullPacketLength(size_t appDataLen) const override;

 protected:
  ReturnValue_t setData(uint8_t* dataPtr, size_t maxSize, void* args = nullptr) override;

  /**
   * A pointer to a structure which defines the data structure of
   * the packet's data.
   *
   * To be hardware-safe, all elements are of byte size.
   */
  TcPacketPointer* tcData = nullptr;
};

#endif /* FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_ */
