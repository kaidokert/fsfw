#include "PusTcReader.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serialize.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PusTcReader::PusTcReader(const uint8_t* data, size_t size) { setData(data, size); }

PusTcReader::~PusTcReader() = default;

ReturnValue_t PusTcReader::parseData() {
  ReturnValue_t result = spReader.checkLength();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (size < PusTcIF::MIN_LEN) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  // Might become variable sized field in the future
  size_t secHeaderLen = ecss::PusTcDataFieldHeader::MIN_LEN;
  pointers.secHeaderStart = pointers.spHeaderStart + ccsds::HEADER_LEN;
  // TODO: No support for spare bytes yet
  pointers.userDataStart = pointers.secHeaderStart + secHeaderLen;
  appDataSize = size - (ccsds::HEADER_LEN + secHeaderLen);
  pointers.crcStart = pointers.userDataStart + appDataSize;
  return HasReturnvaluesIF::RETURN_OK;
}

uint8_t PusTcReader::getAcknowledgeFlags() const {
  return (pointers.secHeaderStart[0] >> 4) & 0b1111;
}

uint8_t PusTcReader::getService() const { return pointers.secHeaderStart[1]; }

uint8_t PusTcReader::getSubService() const { return pointers.secHeaderStart[2]; }
uint16_t PusTcReader::getSourceId() const {
  return (pointers.secHeaderStart[3] << 8) | pointers.secHeaderStart[4];
}
const uint8_t* PusTcReader::getUserData(size_t& appDataLen) const {
  appDataLen = appDataSize;
  return pointers.userDataStart;
}

uint16_t PusTcReader::getUserDataSize() const { return appDataSize; }

uint16_t PusTcReader::getErrorControl() const {
  return pointers.crcStart[0] << 8 | pointers.crcStart[1];
}

uint16_t PusTcReader::getPacketIdRaw() const { return spReader.getPacketIdRaw(); }
uint16_t PusTcReader::getPacketSeqCtrlRaw() const { return spReader.getPacketSeqCtrlRaw(); }
uint16_t PusTcReader::getPacketDataLen() const { return spReader.getPacketDataLen(); }
uint8_t PusTcReader::getPusVersion() const { return spReader.getVersion(); }
const uint8_t* PusTcReader::getFullData() { return pointers.spHeaderStart; }

ReturnValue_t PusTcReader::setData(uint8_t* pData, size_t size_, void* args) {
  size = size_;
  pointers.spHeaderStart = pData;
  spReader.setData(pData, size_, args);
  return HasReturnvaluesIF::RETURN_OK;
}
ReturnValue_t PusTcReader::setData(const uint8_t* data, size_t size_) {
  setData(const_cast<uint8_t*>(data), size_, nullptr);
  return HasReturnvaluesIF::RETURN_OK;
}

/*
void PusTcReader::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "TcPacketBase::print:" << std::endl;
#else
  sif::printInfo("TcPacketBase::print:\n");
#endif
  arrayprinter::print(getWholeData(), getFullSize());
}
*/