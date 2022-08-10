#ifndef FSFW_TESTS_CFDP_USERMOCK_H
#define FSFW_TESTS_CFDP_USERMOCK_H

#include "fsfw/cfdp/handler/UserBase.h"
namespace cfdp {

class UserMock : public UserBase {
  explicit UserMock(HasFileSystemIF& vfs);

 public:
  void transactionIndication(TransactionId id) override;
  void eofSentIndication(TransactionId id) override;
  void abandonedIndication(TransactionId id, ConditionCode code, uint64_t progress) override;
  void eofRecvIndication(TransactionId id) override;
  void transactionFinishedIndication(TransactionFinishedParams params) override;
  void metadataRecvdIndication(MetadataRecvParams params) override;
  void fileSegmentRecvdIndication(FileSegmentRecvdParams params) override;
  void reportIndication() override;
  void suspendedIndication() override;
  void resumedIndication() override;
};

}  // namespace cfdp

#endif  // FSFW_TESTS_CFDP_USERMOCK_H
