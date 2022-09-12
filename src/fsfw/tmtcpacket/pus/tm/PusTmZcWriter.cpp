#include "PusTmZcWriter.h"

#include "fsfw/globalfunctions/CRC.h"

PusTmZeroCopyWriter::PusTmZeroCopyWriter(TimeReaderIF& timeReader, uint8_t* data, size_t size)
    : PusTmReader(&timeReader, data, size) {}

void PusTmZeroCopyWriter::setSequenceCount(uint16_t seqCount) {
  if (isNull()) {
    return;
  }
  auto* spHeader =
      reinterpret_cast<ccsds::PrimaryHeader*>(const_cast<uint8_t*>((pointers.spHeaderStart)));
  ccsds::setSequenceCount(*spHeader, seqCount);
}

void PusTmZeroCopyWriter::updateErrorControl() {
  if (isNull()) {
    return;
  }
  auto* crcStart = const_cast<uint8_t*>((pointers.crcStart));
  uint16_t crc16 =
      CRC::crc16ccitt(PusTmReader::getFullData(), getFullPacketLen() - sizeof(ecss::PusChecksumT));
  crcStart[0] = (crc16 >> 8) & 0xff;
  crcStart[1] = crc16 & 0xff;
}
