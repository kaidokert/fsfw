#include "UserBase.h"

#include <array>
#include <iostream>
void cfdp::testFunc() {
  std::vector<void*> testVec;
  testVec.reserve(10);
  std::array<uint8_t, 4> customMsg = {1, 2, 3, 4};
  MessageToUserTlv tlv0(customMsg.data(), customMsg.size());
  std::cout << testVec.size() << std::endl;
  testVec[0] = &tlv0;
  FilestoreResponseTlv tlv1();
}

cfdp::UserBase::UserBase(HasFileSystemIF& vfs) : vfs(vfs) {}
