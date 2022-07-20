#ifndef FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_

#include <cstdint>

#include "../definitions.h"
#include "PusTcReader.h"
#include "fsfw/FSFW.h"
#include "fsfw/tmtcpacket/ccsds/defs.h"

/**
 * This struct defines the data structure of a PUS Telecommand A packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
/*
struct TcPacketPointer {
 CCSDSPrimaryHeader primary;
 PusTcDataFieldHeader dataField;
 uint8_t appData;
};
 */

// class TcPacketPus : public TcPacketPusBase {
//  public:
//   /**
//    * Initialize a PUS A telecommand packet which already exists. You can also
//    * create an empty (invalid) object by passing nullptr as the data pointer
//    * @param setData
//    */
//   TcPacketPus(const uint8_t* setData);
//
//   // Base class overrides
//   uint8_t getSecondaryHeaderFlag() const override;
//   uint8_t getPusVersionNumber() const override;
//   uint8_t getAcknowledgeFlags() const override;
//   uint8_t getService() const override;
//   uint8_t getSubService() const override;
//   uint16_t getSourceId() const override;
//   const uint8_t* getApplicationData() const override;
//   uint16_t getApplicationDataSize() const override;
//   uint16_t getErrorControl() const override;
//   void setErrorControl() override;
//   size_t calculateFullPacketLength(size_t appDataLen) const override;
//
//  protected:
//   ReturnValue_t setData(uint8_t* dataPtr, size_t maxSize, void* args) override;
//
//   /**
//    * Initializes the Tc Packet header.
//    * @param apid APID used.
//    * @param sequenceCount Sequence Count in the primary header.
//    * @param ack Which acknowledeges are expected from the receiver.
//    * @param service   PUS Service
//    * @param subservice PUS Subservice
//    */
//   void initializeTcPacket(uint16_t apid, uint16_t sequenceCount, uint8_t ack, uint8_t service,
//                           uint8_t subservice, pus::PusVersion pusVersion, uint16_t sourceId = 0);
//
//   /**
//    * A pointer to a structure which defines the data structure of
//    * the packet's data.
//    *
//    * To be hardware-safe, all elements are of byte size.
//    */
//   TcPacketPointer* tcData = nullptr;
// };

#endif /* FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_ */
