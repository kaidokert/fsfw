#include "PusTmReader.h"

#include "fsfw/globalfunctions/CRC.h"

PusTmReader::PusTmReader(TimeReaderIF *timeReader) : timeReader(timeReader) {}

PusTmReader::PusTmReader(const uint8_t *data, size_t size) { setReadOnlyData(data, size); }

PusTmReader::PusTmReader(TimeReaderIF *timeReader, const uint8_t *data, size_t size)
    : PusTmReader(timeReader) {
  setReadOnlyData(data, size);
}

ReturnValue_t PusTmReader::parseDataWithCrcCheck() { return parseData(true); }

ReturnValue_t PusTmReader::parseDataWithoutCrcCheck() { return parseData(false); }

const uint8_t *PusTmReader::getFullData() { return spReader.getFullData(); }

ReturnValue_t PusTmReader::setReadOnlyData(const uint8_t *data, size_t size) {
  return setData(const_cast<uint8_t *>(data), size, nullptr);
}

ReturnValue_t PusTmReader::setData(uint8_t *dataPtr, size_t size, void *args) {
  pointers.spHeaderStart = dataPtr;
  return spReader.setReadOnlyData(dataPtr, size);
}

uint16_t PusTmReader::getPacketIdRaw() const { return spReader.getPacketIdRaw(); }
uint16_t PusTmReader::getPacketSeqCtrlRaw() const { return spReader.getPacketSeqCtrlRaw(); }
uint16_t PusTmReader::getPacketDataLen() const { return spReader.getPacketDataLen(); }
uint8_t PusTmReader::getPusVersion() const { return (pointers.secHeaderStart[0] >> 4) & 0b1111; }
uint8_t PusTmReader::getScTimeRefStatus() { return pointers.secHeaderStart[0] & 0b1111; }
uint8_t PusTmReader::getService() const { return pointers.secHeaderStart[1]; }
uint8_t PusTmReader::getSubService() const { return pointers.secHeaderStart[2]; }
const uint8_t *PusTmReader::getUserData() const { return pointers.userDataStart; }
size_t PusTmReader::getUserDataLen() const { return sourceDataLen; }

uint16_t PusTmReader::getMessageTypeCounter() {
  return (pointers.secHeaderStart[3] << 8) | pointers.secHeaderStart[4];
}

uint16_t PusTmReader::getDestId() {
  return (pointers.secHeaderStart[5] << 8) | pointers.secHeaderStart[6];
}

void PusTmReader::setTimeReader(TimeReaderIF *timeReader_) { timeReader = timeReader_; }

TimeReaderIF *PusTmReader::getTimeReader() { return timeReader; }

ReturnValue_t PusTmReader::parseData(bool crcCheck) {
  // Time reader is required to read the time stamp length at run-time
  if (pointers.spHeaderStart == nullptr or spReader.isNull() or timeReader == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  ReturnValue_t result = spReader.checkSize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (spReader.getFullPacketLen() < spReader.getBufSize()) {
    return SerializeIF::STREAM_TOO_SHORT;
  }

  size_t currentOffset = SpacePacketReader::getHeaderLen();
  pointers.secHeaderStart = pointers.spHeaderStart + currentOffset;
  currentOffset += PusTmIF::MIN_SEC_HEADER_LEN;
  size_t minTimestampLen = spReader.getBufSize() - currentOffset;
  result = timeReader->readTimeStamp(pointers.secHeaderStart + PusTmIF::MIN_SEC_HEADER_LEN,
                                     minTimestampLen);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t timestampLen = timeReader->getTimestampLen();
  if (currentOffset + timestampLen > spReader.getBufSize()) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  currentOffset += timestampLen;
  pointers.userDataStart = pointers.spHeaderStart + currentOffset;
  sourceDataLen = spReader.getFullPacketLen() - currentOffset - sizeof(ecss::PusChecksumT);
  currentOffset += sourceDataLen;
  pointers.crcStart = pointers.spHeaderStart + currentOffset;
  if (crcCheck) {
    uint16_t crc16 = CRC::crc16ccitt(spReader.getFullData(), getFullPacketLen());
    if (crc16 != 0) {
      // Checksum failure
      return PusIF::INVALID_CRC_16;
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}
