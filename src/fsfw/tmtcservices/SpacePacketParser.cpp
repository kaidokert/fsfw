#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tmtcservices/SpacePacketParser.h>

#include <algorithm>

SpacePacketParser::SpacePacketParser(std::vector<uint16_t> validPacketIds)
    : validPacketIds(validPacketIds) {}

ReturnValue_t SpacePacketParser::parseSpacePackets(const uint8_t** buffer, const size_t maxSize,
                                                   FoundPacketInfo& packetInfo) {
  if (buffer == nullptr or nextStartIdx > maxSize) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "SpacePacketParser::parseSpacePackets: Frame invalid" << std::endl;
#else
    sif::printWarning("SpacePacketParser::parseSpacePackets: Frame invalid\n");
#endif
    return returnvalue::FAILED;
  }
  const uint8_t* bufPtr = *buffer;

  auto verifyLengthField = [&](size_t localIdx) {
    uint16_t lengthField = (bufPtr[localIdx + 4] << 8) | bufPtr[localIdx + 5];
    size_t packetSize = lengthField + 7;
    ReturnValue_t result = returnvalue::OK;
    if (packetSize + localIdx + amountRead > maxSize) {
      // Don't increment buffer and read length here, user has to decide what to do
      packetInfo.sizeFound = packetSize;
      return SPLIT_PACKET;
    } else {
      packetInfo.sizeFound = packetSize;
    }
    *buffer += packetInfo.sizeFound;
    packetInfo.startIdx = localIdx + amountRead;
    nextStartIdx = localIdx + amountRead + packetInfo.sizeFound;
    amountRead = nextStartIdx;
    return result;
  };

  size_t idx = 0;
  // Space packet ID as start marker
  if (validPacketIds.size() > 0) {
    while (idx + amountRead < maxSize - 5) {
      uint16_t currentPacketId = (bufPtr[idx] << 8) | bufPtr[idx + 1];
      if (std::find(validPacketIds.begin(), validPacketIds.end(), currentPacketId) !=
          validPacketIds.end()) {
        if (idx + amountRead >= maxSize - 5) {
          return SPLIT_PACKET;
        }
        return verifyLengthField(idx);
      } else {
        idx++;
      }
    }
    nextStartIdx = maxSize;
    packetInfo.sizeFound = maxSize;
    amountRead = maxSize;
    *buffer += maxSize;
    return NO_PACKET_FOUND;
  }
  // Assume that the user verified a valid start of a space packet
  else {
    return verifyLengthField(idx);
  }
}
