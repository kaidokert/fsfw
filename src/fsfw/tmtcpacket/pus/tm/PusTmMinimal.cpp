#include "fsfw/tmtcpacket/pus/tm/PusTmMinimal.h"

#include <cstddef>
#include <ctime>

#include "fsfw/tmtcpacket/pus/PacketTimestampInterpreterIF.h"

PusTmMinimal::PusTmMinimal(mintm::MinimalPusTm* data) { tmData = data; }
PusTmMinimal::PusTmMinimal(uint8_t* data) {
  this->tmData = reinterpret_cast<mintm::MinimalPusTm*>(data);
}

PusTmMinimal::~PusTmMinimal() = default;

ReturnValue_t PusTmMinimal::getPacketTime(timeval* timestamp) {
  if (timestampInterpreter == nullptr) {
    return returnvalue::FAILED;
  }
  return timestampInterpreter->getPacketTime(this, timestamp);
}

void PusTmMinimal::setInterpretTimestampObject(PacketTimestampInterpreterIF* interpreter) {
  if (PusTmMinimal::timestampInterpreter == nullptr) {
    PusTmMinimal::timestampInterpreter = interpreter;
  }
}

PacketTimestampInterpreterIF* PusTmMinimal::timestampInterpreter = nullptr;
// TODO: Implement all of this
ReturnValue_t PusTmMinimal::setData(uint8_t* dataPtr, size_t size, void* args) { return 0; }
uint16_t PusTmMinimal::getPacketIdRaw() const { return 0; }
uint16_t PusTmMinimal::getPacketSeqCtrlRaw() const { return 0; }
uint16_t PusTmMinimal::getPacketDataLen() const { return 0; }
uint8_t PusTmMinimal::getPusVersion() const { return 0; }
uint8_t PusTmMinimal::getService() const { return tmData->secHeader.service; }
uint8_t PusTmMinimal::getSubService() const { return tmData->secHeader.subservice; }
uint8_t PusTmMinimal::getScTimeRefStatus() { return 0; }
uint16_t PusTmMinimal::getMessageTypeCounter() {
  return (tmData->secHeader.messageTypeH << 8) | tmData->secHeader.messageTypeL;
}
uint16_t PusTmMinimal::getDestId() { return 0; }
void PusTmMinimal::setApid(uint16_t apid) {
  /* TODO: Implement. Maybe provide low level function to do this */
}
size_t PusTmMinimal::getUserDataLen() const { return userDataLen; }

const uint8_t* PusTmMinimal::getUserData() const {
  return reinterpret_cast<const uint8_t*>(&tmData->rest);
}
