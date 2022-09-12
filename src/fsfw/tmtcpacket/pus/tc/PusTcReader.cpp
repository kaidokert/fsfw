#include "PusTcReader.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serialize.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PusTcReader::PusTcReader(const uint8_t* data, size_t size) { setReadOnlyData(data, size); }

PusTcReader::~PusTcReader() = default;

ReturnValue_t PusTcReader::parseDataWithCrcCheck() { return parseData(true); }

ReturnValue_t PusTcReader::parseDataWithoutCrcCheck() { return parseData(false); }

ReturnValue_t PusTcReader::parseData(bool withCrc) {
  if (pointers.spHeaderStart == nullptr or spReader.isNull()) {
    return returnvalue::FAILED;
  }
  ReturnValue_t result = spReader.checkSize();
  if (result != returnvalue::OK) {
    return result;
  }
  size_t currentOffset = SpacePacketReader::getHeaderLen();
  pointers.secHeaderStart = pointers.spHeaderStart + currentOffset;
  // Might become variable sized field in the future
  // TODO: No support for spare bytes yet
  currentOffset += ecss::PusTcDataFieldHeader::MIN_SIZE;
  appDataSize = spReader.getFullPacketLen() - currentOffset - sizeof(ecss::PusChecksumT);
  pointers.userDataStart = nullptr;
  if (appDataSize > 0) {
    pointers.userDataStart = pointers.spHeaderStart + currentOffset;
  }
  currentOffset += appDataSize;
  pointers.crcStart = pointers.spHeaderStart + currentOffset;
  if (withCrc) {
    uint16_t crc16 = CRC::crc16ccitt(spReader.getFullData(), getFullPacketLen());
    if (crc16 != 0) {
      // Checksum failure
      return PusIF::INVALID_CRC_16;
    }
  }
  return returnvalue::OK;
}

uint8_t PusTcReader::getPusVersion() const { return (pointers.secHeaderStart[0] >> 4) & 0b1111; }

uint8_t PusTcReader::getAcknowledgeFlags() const { return pointers.secHeaderStart[0] & 0b1111; }

uint8_t PusTcReader::getService() const { return pointers.secHeaderStart[1]; }

uint8_t PusTcReader::getSubService() const { return pointers.secHeaderStart[2]; }

uint16_t PusTcReader::getSourceId() const {
  return (pointers.secHeaderStart[3] << 8) | pointers.secHeaderStart[4];
}

uint16_t PusTcReader::getErrorControl() const {
  return (pointers.crcStart[0] << 8) | pointers.crcStart[1];
}

uint16_t PusTcReader::getPacketIdRaw() const { return spReader.getPacketIdRaw(); }

uint16_t PusTcReader::getPacketSeqCtrlRaw() const { return spReader.getPacketSeqCtrlRaw(); }

uint16_t PusTcReader::getPacketDataLen() const { return spReader.getPacketDataLen(); }

const uint8_t* PusTcReader::getFullData() const { return pointers.spHeaderStart; }

ReturnValue_t PusTcReader::setData(uint8_t* pData, size_t size_, void* args) {
  pointers.spHeaderStart = pData;
  return spReader.setReadOnlyData(pData, size_);
}

ReturnValue_t PusTcReader::setReadOnlyData(const uint8_t* data, size_t size_) {
  return setData(const_cast<uint8_t*>(data), size_, nullptr);
}

const uint8_t* PusTcReader::getUserData() const { return pointers.userDataStart; }
size_t PusTcReader::getUserDataLen() const { return appDataSize; }

bool PusTcReader::isNull() const { return spReader.isNull(); }

PusTcReader::operator bool() const { return not isNull(); }
