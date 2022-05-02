#include "fsfw/tmtcpacket/SpacePacket.h"

#include <cstring>

#include "fsfw/tmtcpacket/ccsds_header.h"

SpacePacket::SpacePacket(uint16_t packetDataLength, bool isTelecommand, uint16_t apid,
                         uint16_t sequenceCount)
    : SpacePacketBase((uint8_t*)&this->localData) {
  initSpacePacketHeader(isTelecommand, false, apid, sequenceCount);
  this->setPacketSequenceCount(sequenceCount);
  if (packetDataLength <= sizeof(this->localData.fields.buffer)) {
    this->setPacketDataLength(packetDataLength);
  } else {
    this->setPacketDataLength(sizeof(this->localData.fields.buffer));
  }
}

SpacePacket::~SpacePacket(void) {}

bool SpacePacket::addWholeData(const uint8_t* p_Data, uint32_t packet_size) {
  if (packet_size <= sizeof(this->localData)) {
    memcpy(this->localData.byteStream, p_Data, packet_size);
    return true;
  } else {
    return false;
  }
}
