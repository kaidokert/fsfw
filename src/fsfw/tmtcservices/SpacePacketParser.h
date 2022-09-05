#ifndef FRAMEWORK_TMTCSERVICES_PUSPARSER_H_
#define FRAMEWORK_TMTCSERVICES_PUSPARSER_H_

#include <cstdint>
#include <utility>

#include "fsfw/container/DynamicFIFO.h"
#include "fsfw/returnvalues/FwClassIds.h"

/**
 * @brief	This small helper class scans a given buffer for space packets.
 * 			Can be used if space packets are serialized in a tightly packed frame.
 * @details
 * The parser uses the length field field and the 16-bit TC packet ID of the space packets to find
 * find space packets in a given data stream
 * @author   R. Mueller
 */
class SpacePacketParser {
 public:
  struct FoundPacketInfo {
    size_t startIdx = 0;
    size_t sizeFound = 0;
  };

  static constexpr uint8_t INTERFACE_ID = CLASS_ID::SPACE_PACKET_PARSER;
  static constexpr ReturnValue_t NO_PACKET_FOUND = MAKE_RETURN_CODE(0x00);
  static constexpr ReturnValue_t SPLIT_PACKET = MAKE_RETURN_CODE(0x01);

  /**
   * @brief   Parser constructor.
   * @param validPacketIds    This vector contains the allowed 16-bit TC packet ID start markers
   * The parser will search for these stark markers to detect the start of a space packet.
   * It is also possible to pass an empty vector here, but this is not recommended.
   * If an empty vector is passed, the parser will assume that the start of the given stream
   * contains the start of a new space packet.
   */
  SpacePacketParser(std::vector<uint16_t> validPacketIds);

  /**
   * Parse a given frame for space packets but also increments the given buffer.
   * @param buffer        Parser will look for space packets in this buffer
   * @param maxSize       Maximum size of the buffer
   * @param packetInfo    Information about packets found.
   *  -@c NO_PACKET_FOUND if no packet was found in the given buffer
   *  -@c SPLIT_PACKET if a packet was found but the detected size exceeds maxSize. packetInfo
   *      will contain the detected packet size and start index.
   *  -@c returnvalue::OK if a packet was found. Packet size and start index will be set in
   *      packetInfo
   */
  ReturnValue_t parseSpacePackets(const uint8_t** buffer, const size_t maxSize,
                                  FoundPacketInfo& packetInfo);

  size_t getAmountRead() { return amountRead; }

  void reset() {
    nextStartIdx = 0;
    amountRead = 0;
  }

 private:
  std::vector<uint16_t> validPacketIds;
  size_t nextStartIdx = 0;
  size_t amountRead = 0;
};

#endif /* FRAMEWORK_TMTCSERVICES_PUSPARSER_H_ */
