#include "header.h"

uint16_t ccsds::getPacketId(const PrimaryHeader &header) {
  return ((header.packetIdHAndVersion & 0x1F) << 8) + header.packetIdL;
}

uint16_t ccsds::getPacketSeqCtrl(const PrimaryHeader &header) {
  return (header.packetSeqCtrlH << 8) | header.packetSeqCtrlL;
}

uint8_t ccsds::getVersion(const PrimaryHeader &header) {
  return (header.packetIdHAndVersion >> 5) & 0b111;
}

uint16_t ccsds::getPacketLen(const PrimaryHeader &header) {
  return (header.packetLenH << 8) | header.packetLenL;
}

void ccsds::setPacketId(PrimaryHeader &header, uint16_t packetId) {
  header.packetIdHAndVersion |= (packetId >> 8) & 0x1F;
  header.packetIdL = packetId & 0xff;
}

void ccsds::setApid(PrimaryHeader &header, uint16_t apid) {
  uint16_t packetIdRaw = getPacketId(header);
  packetIdRaw &= ~0x7FF;
  packetIdRaw |= apid & 0x7FF;
  setPacketId(header, packetIdRaw);
}

void ccsds::setSequenceCount(PrimaryHeader &header, uint16_t seqCount) {
  header.packetSeqCtrlH |= (seqCount >> 8) & 0x3F;
  header.packetSeqCtrlL = seqCount & 0xFF;
}
