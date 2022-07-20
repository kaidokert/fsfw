#include "SpacePacketReader.h"

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

inline uint16_t SpacePacketReader::getPacketIdRaw() const { return ccsds::getPacketId(*spHeader); }

const uint8_t* SpacePacketReader::getPacketData() { return packetDataField; }

ReturnValue_t SpacePacketReader::setData(uint8_t* pData, size_t maxSize_, void* args) {
  setInternalFields(pData, maxSize_);
  return HasReturnvaluesIF::RETURN_OK;
}

uint16_t SpacePacketReader::getPacketSeqCtrlRaw() const {
  return ccsds::getPacketSeqCtrl(*spHeader);
}

uint16_t SpacePacketReader::getPacketDataLen() const { return ccsds::getPacketLen(*spHeader); }
void SpacePacketReader::setInternalFields(const uint8_t* data, size_t maxSize_) {
  maxSize = maxSize_;
  spHeader = reinterpret_cast<const ccsds::PrimaryHeader*>(data);
  packetDataField = data + ccsds::HEADER_LEN;
}
const uint8_t* SpacePacketReader::getFullData() {
  return reinterpret_cast<const uint8_t*>(spHeader);
};
