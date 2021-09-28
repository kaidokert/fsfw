#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tmtcservices/SpacePacketParser.h>
#include <algorithm>

SpacePacketParser::SpacePacketParser(std::vector<uint16_t> validPacketIds):
        validPacketIds(validPacketIds) {
}

ReturnValue_t SpacePacketParser::parseSpacePackets(const uint8_t *buffer,
        const size_t maxSize, size_t& startIndex, size_t& foundSize) {
    const uint8_t** tempPtr = &buffer;
    size_t readLen = 0;
    return parseSpacePackets(tempPtr, maxSize, startIndex, foundSize, readLen);
}

ReturnValue_t SpacePacketParser::parseSpacePackets(const uint8_t **buffer, const size_t maxSize,
        size_t &startIndex, size_t &foundSize, size_t& readLen) {
    if(buffer == nullptr or maxSize < 5) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SpacePacketParser::parseSpacePackets: Frame invalid" << std::endl;
#else
        sif::printWarning("SpacePacketParser::parseSpacePackets: Frame invalid\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    const uint8_t* bufPtr = *buffer;

    auto verifyLengthField = [&](size_t idx) {
        uint16_t lengthField = bufPtr[idx + 4] << 8 | bufPtr[idx + 5];
        size_t packetSize = lengthField + 7;
        startIndex = idx;
        ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
        if(lengthField == 0) {
            // Skip whole header for now
            foundSize = 6;
            result = NO_PACKET_FOUND;
        }
        else if(packetSize + idx > maxSize) {
            // Don't increment buffer and read length here, user has to decide what to do
            foundSize = packetSize;
            return SPLIT_PACKET;
        }
        else {
            foundSize = packetSize;
        }
        *buffer += foundSize;
        readLen += idx + foundSize;
        return result;
    };

    size_t idx = 0;
    // Space packet ID as start marker
    if(validPacketIds.size() > 0) {
        while(idx < maxSize - 5) {
            uint16_t currentPacketId = bufPtr[idx] << 8 | bufPtr[idx + 1];
            if(std::find(validPacketIds.begin(), validPacketIds.end(), currentPacketId) !=
                    validPacketIds.end()) {
                if(idx + 5 >= maxSize) {
                    return SPLIT_PACKET;
                }
                return verifyLengthField(idx);
            }
            else {
                idx++;
            }
        }
        startIndex = 0;
        foundSize = maxSize;
        *buffer += foundSize;
        readLen += foundSize;
        return NO_PACKET_FOUND;
    }
    // Assume that the user verified a valid start of a space packet
    else {
        return verifyLengthField(idx);
    }
}
