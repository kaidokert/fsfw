#include "PusTcReader.h"

#include <cstring>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serialize.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PusTcReader::PusTcReader(const uint8_t* setData, size_t size) : spReader(setData), maxSize(size) {
  pointers.spHeaderStart = setData;
}

PusTcReader::~PusTcReader() = default;

ReturnValue_t PusTcReader::parseData() {
  if (maxSize < sizeof(CCSDSPrimaryHeader)) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  pointers.secHeaderStart = pointers.spHeaderStart + sizeof(CCSDSPrimaryHeader);
  // TODO: No support for spare bytes yet
  pointers.userDataStart = pointers.secHeaderStart + ecss::PusTcDataFieldHeader::MIN_LEN;
}

uint8_t PusTcReader::getAcknowledgeFlags() const { return 0; }
uint8_t PusTcReader::getService() const { return 0; }
uint8_t PusTcReader::getSubService() const { return 0; }
uint16_t PusTcReader::getSourceId() const { return 0; }
const uint8_t* PusTcReader::getApplicationData() const { return nullptr; }
uint16_t PusTcReader::getApplicationDataSize() const { return 0; }
uint16_t PusTcReader::getErrorControl() const { return 0; }

uint16_t PusTcReader::getPacketId() const { return spReader.getPacketId(); }
uint16_t PusTcReader::getPacketSeqCtrl() const { return spReader.getPacketSeqCtrl(); }
uint16_t PusTcReader::getPacketDataLen() const { return spReader.getPacketDataLen(); }
uint8_t PusTcReader::getPusVersion() const { return spReader.getVersion(); }

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