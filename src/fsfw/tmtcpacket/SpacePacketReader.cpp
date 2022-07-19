#include "fsfw/tmtcpacket/SpacePacketReader.h"

#include <cstring>

#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

SpacePacketReader::SpacePacketReader(const uint8_t* setAddress, size_t maxSize_) {
  setInternalFields(setAddress, maxSize_);
}

ReturnValue_t SpacePacketReader::checkLength() const {
  if (getFullPacketLen() > maxSize) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

SpacePacketReader::~SpacePacketReader() = default;

inline uint16_t SpacePacketReader::getPacketId() const {
  return ((spHeader->packetIdHAndVersion) << 8) + spHeader->packetIdL;
}

const uint8_t* SpacePacketReader::getPacketData() { return packetDataField; }

ReturnValue_t SpacePacketReader::setData(uint8_t* pData, size_t maxSize_, void* args) {
  setInternalFields(pData, maxSize_);
}

uint16_t SpacePacketReader::getPacketSeqCtrl() const {
  return (spHeader->packetSeqCtrlH << 8) + spHeader->packetSeqCtrlL;
}

uint16_t SpacePacketReader::getPacketDataLen() const {
  return (spHeader->packetLenH << 8) | spHeader->packetIdL;
}
void SpacePacketReader::setInternalFields(const uint8_t* data, size_t maxSize_) {
  maxSize = maxSize_;
  spHeader = reinterpret_cast<const CCSDSPrimaryHeader*>(data);
  packetDataField = data + sizeof(CCSDSPrimaryHeader);
}
const uint8_t* SpacePacketReader::getFullData() {
  return reinterpret_cast<const uint8_t*>(spHeader);
};
