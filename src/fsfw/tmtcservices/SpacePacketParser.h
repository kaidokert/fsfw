#ifndef FRAMEWORK_TMTCSERVICES_PUSPARSER_H_
#define FRAMEWORK_TMTCSERVICES_PUSPARSER_H_

#include <cstdint>
#include <utility>

#include "fsfw/container/DynamicFIFO.h"
#include "fsfw/returnvalues/FwClassIds.h"

struct FoundPacketInfo {
  size_t startIdx = 0;
  size_t sizeFound = 0;
};

struct ParsingState {
  size_t nextStartIdx = 0;
  size_t amountRead = 0;
};

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
  //! The first entry is the index inside the buffer while the second index
  //! is the size of the PUS packet starting at that index.
  using IndexSizePair = std::pair<size_t, size_t>;

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
   * Parse a given frame for space packets but also increment the given buffer and assign the
   * total number of bytes read so far
   * @param buffer        Parser will look for space packets in this buffer
   * @param maxSize       Maximum size of the buffer
   * @param startIndex    Start index of a found space packet
   * @param foundSize     Found size of the space packet
   * @param readLen       Length read so far. This value is incremented by the number of parsed
   *                      bytes which also includes the size of a found packet
   *  -@c NO_PACKET_FOUND if no packet was found in the given buffer or the length field is
   *      invalid. foundSize will be set to the size of the space packet header. buffer and
   *      readLen will be incremented accordingly.
   *  -@c SPLIT_PACKET if a packet was found but the detected size exceeds maxSize. foundSize
   *      will be set to the detected packet size and startIndex will be set to the start of the
   *      detected packet. buffer and read length will not be incremented but the found length
   *      will be assigned.
   *  -@c returnvalue::OK if a packet was found
   */
  ReturnValue_t parseSpacePackets(const uint8_t** buffer, const size_t maxSize,
                                  FoundPacketInfo& packetInfo, ParsingState& parsingState);

  /**
   * Parse a given frame for space packets
   * @param buffer        Parser will look for space packets in this buffer
   * @param maxSize       Maximum size of the buffer
   * @param startIndex    Start index of a found space packet
   * @param foundSize     Found size of the space packet
   *  -@c NO_PACKET_FOUND if no packet was found in the given buffer or the length field is
   *      invalid. foundSize will be set to the size of the space packet header
   *  -@c SPLIT_PACKET if a packet was found but the detected size exceeds maxSize. foundSize
   *      will be set to the detected packet size and startIndex will be set to the start of the
   *      detected packet
   *  -@c returnvalue::OK if a packet was found
   */
  ReturnValue_t parseSpacePackets(const uint8_t* buffer, const size_t maxSize,
                                  FoundPacketInfo& packetInfo, ParsingState& parsingState);

 private:
  std::vector<uint16_t> validPacketIds;
};

#endif /* FRAMEWORK_TMTCSERVICES_PUSPARSER_H_ */
