#include "fsfw/tcdistribution/PusPacketChecker.h"

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/definitions.h"
#include "fsfw/tmtcpacket/pus/tc/PusTcReader.h"

PusPacketChecker::PusPacketChecker(uint16_t setApid, ccsds::PacketType packetType_,
                                   ecss::PusVersion pusVersion_)
    : CcsdsPacketCheckerBase(setApid, packetType_), pusVersion(pusVersion_) {}

ReturnValue_t PusPacketChecker::checkPacket(PacketCheckIF* pusPacket, size_t packetLen) {
  if (pusPacket == nullptr) {
    return RETURN_FAILED;
  }
  // Other primary header fields are checked by base class
  if (not pusPacket->hasSecHeader()) {
    return tcdistrib::INVALID_SEC_HEADER_FIELD;
  }
  uint16_t calculated_crc =
      CRC::crc16ccitt(pusPacket->getFullData(), pusPacket->getFullPacketLen());
  if (calculated_crc != 0) {
    return tcdistrib::INCORRECT_CHECKSUM;
  }

  // This assumes that the getFullPacketLen version uses the space packet data length field
  if (pusPacket->getFullPacketLen() != packetLen) {
    return tcdistrib::INCOMPLETE_PACKET;
  }
  if (pusPacket->getPusVersion() != pusVersion) {
    return tcdistrib::INVALID_PUS_VERSION;
  }
  return RETURN_OK;
}

uint16_t PusPacketChecker::getApid() const { return apid; }

ReturnValue_t PusPacketChecker::checkPacket(SpacePacketReader* currentPacket, size_t packetLen) {
  ReturnValue_t result = CcsdsPacketCheckerBase::checkPacket(currentPacket, packetLen);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return checkPacket(dynamic_cast<PacketCheckIF*>(currentPacket), packetLen);
}
