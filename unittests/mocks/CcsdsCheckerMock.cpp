#include "CcsdsCheckerMock.h"

CcsdsCheckerMock::CcsdsCheckerMock() = default;

ReturnValue_t CcsdsCheckerMock::checkPacket(const SpacePacketReader& currentPacket,
                                            size_t packetLen) {
  checkCallCount++;
  checkedPacketLen = packetLen;
  return nextResult;
}
