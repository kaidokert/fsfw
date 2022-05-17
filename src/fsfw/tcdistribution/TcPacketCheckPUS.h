#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_

#include "TcPacketCheckIF.h"
#include "fsfw/FSFW.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

class TcPacketStoredBase;

/**
 * This class performs a formal packet check for incoming PUS Telecommand Packets.
 * Currently, it only checks if the APID and CRC are correct.
 * @ingroup tc_distribution
 */
class TcPacketCheckPUS : public TcPacketCheckIF, public HasReturnvaluesIF {
 protected:
  /**
   * Describes the version number a packet must have to pass.
   */
  static constexpr uint8_t CCSDS_VERSION_NUMBER = 0;
  /**
   * Describes the secondary header a packet must have to pass.
   */
  static constexpr uint8_t CCSDS_SECONDARY_HEADER_FLAG = 0;
  /**
   * Describes the TC Packet PUS Version Number a packet must have to pass.
   */
#if FSFW_USE_PUS_C_TELECOMMANDS == 1
  static constexpr uint8_t PUS_VERSION_NUMBER = 2;
#else
  static constexpr uint8_t PUS_VERSION_NUMBER = 1;
#endif

  /**
   * The packet id each correct packet should have.
   * It is composed of the APID and some static fields.
   */
  uint16_t apid;

 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::TC_PACKET_CHECK;
  static const ReturnValue_t ILLEGAL_APID = MAKE_RETURN_CODE(0);
  static const ReturnValue_t INCOMPLETE_PACKET = MAKE_RETURN_CODE(1);
  static const ReturnValue_t INCORRECT_CHECKSUM = MAKE_RETURN_CODE(2);
  static const ReturnValue_t ILLEGAL_PACKET_TYPE = MAKE_RETURN_CODE(3);
  static const ReturnValue_t ILLEGAL_PACKET_SUBTYPE = MAKE_RETURN_CODE(4);
  static const ReturnValue_t INCORRECT_PRIMARY_HEADER = MAKE_RETURN_CODE(5);
  static const ReturnValue_t INCORRECT_SECONDARY_HEADER = MAKE_RETURN_CODE(6);
  /**
   * The constructor only sets the APID attribute.
   * @param set_apid The APID to set.
   */
  TcPacketCheckPUS(uint16_t setApid);

  ReturnValue_t checkPacket(SpacePacketBase* currentPacket) override;

  uint16_t getApid() const;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKPUS_H_ */
