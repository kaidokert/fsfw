#include "PusTmZcWriter.h"

#include "fsfw/globalfunctions/CRC.h"

PusTmZeroCopyWriter::PusTmZeroCopyWriter(uint8_t* data, size_t size) : PusTmReader(data, size) {}

void PusTmZeroCopyWriter::setSequenceCount(uint16_t seqCount) {
  auto* spHeader =
      reinterpret_cast<ccsds::PrimaryHeader*>(const_cast<uint8_t*>((pointers.spHeaderStart)));
  ccsds::setSequenceCount(*spHeader, seqCount);
}
void PusTmZeroCopyWriter::updateErrorControl() {
  auto* crcStart = reinterpret_cast<uint8_t*>(const_cast<uint8_t*>((pointers.spHeaderStart)));
  uint16_t crc16 =
      CRC::crc16ccitt(PusTmReader::getFullData(), getFullPacketLen() - sizeof(ecss::PusChecksumT));
  crcStart[0] = (crc16 >> 8) & 0xff;
  crcStart[1] = crc16 & 0xff;
}