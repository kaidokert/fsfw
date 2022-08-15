#include "SpacePacketReader.h"

#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

SpacePacketReader::SpacePacketReader(const uint8_t* setAddress, size_t maxSize_) {
  setInternalFields(setAddress, maxSize_);
}

ReturnValue_t SpacePacketReader::checkSize() const {
  if (isNull()) {
    return returnvalue::FAILED;
  }
  if (getFullPacketLen() > bufSize) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  return returnvalue::OK;
}

SpacePacketReader::~SpacePacketReader() = default;

inline uint16_t SpacePacketReader::getPacketIdRaw() const { return ccsds::getPacketId(*spHeader); }

const uint8_t* SpacePacketReader::getPacketData() { return packetDataField; }

ReturnValue_t SpacePacketReader::setData(uint8_t* data, size_t maxSize_, void* args) {
  return setInternalFields(data, maxSize_);
}

uint16_t SpacePacketReader::getPacketSeqCtrlRaw() const {
  return ccsds::getPacketSeqCtrl(*spHeader);
}

uint16_t SpacePacketReader::getPacketDataLen() const { return ccsds::getPacketLen(*spHeader); }

ReturnValue_t SpacePacketReader::setInternalFields(const uint8_t* data, size_t maxSize_) {
  bufSize = maxSize_;
  spHeader = reinterpret_cast<const ccsds::PrimaryHeader*>(data);
  if (maxSize_ > 6) {
    packetDataField = data + ccsds::HEADER_LEN;
  }
  return checkSize();
}

const uint8_t* SpacePacketReader::getFullData() const {
  return reinterpret_cast<const uint8_t*>(spHeader);
}
size_t SpacePacketReader::getBufSize() const { return bufSize; }

bool SpacePacketReader::isNull() const { return spHeader == nullptr; }

ReturnValue_t SpacePacketReader::setReadOnlyData(const uint8_t* data, size_t maxSize) {
  return setData(const_cast<uint8_t*>(data), maxSize, nullptr);
}
