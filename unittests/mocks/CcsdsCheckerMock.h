#ifndef FSFW_TESTS_CCSDSCHECKERMOCK_H
#define FSFW_TESTS_CCSDSCHECKERMOCK_H

#include "fsfw/tcdistribution/CcsdsPacketCheckIF.h"
class CcsdsCheckerMock : public CcsdsPacketCheckIF {
 public:
  CcsdsCheckerMock();
  unsigned int checkCallCount = 0;
  size_t checkedPacketLen = 0;
  ReturnValue_t nextResult = returnvalue::OK;
  ReturnValue_t checkPacket(const SpacePacketReader& currentPacket, size_t packetLen) override;

 private:
};

#endif  // FSFW_TESTS_CCSDSCHECKERMOCK_H
