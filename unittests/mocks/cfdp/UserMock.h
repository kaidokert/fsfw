#ifndef FSFW_TESTS_CFDP_USERMOCK_H
#define FSFW_TESTS_CFDP_USERMOCK_H

#include "fsfw/cfdp/handler/UserBase.h"
namespace cfdp {

class UserMock : public UserBase {
  explicit UserMock(HasFileSystemIF& vfs);

 public:
  void transactionIndication(TransactionId id) override;
  void eofSentIndication(TransactionId id) override;
  void abandonedIndication(TransactionId id, ConditionCode code, size_t progress) override;
  void eofRecvIndication(TransactionId id) override;
  void transactionFinishedIndication(TransactionFinishedParams params) override;
  void metadataRecvdIndication(MetadataRecvdParams params) override;
  void fileSegmentRecvdIndication(FileSegmentRecvdParams params) override;
  void reportIndication(TransactionId id, StatusReportIF& report) override;
  void suspendedIndication(TransactionId id, ConditionCode code) override;
  void resumedIndication(TransactionId id, size_t progress) override;
  void faultIndication(TransactionId id, ConditionCode code, size_t progress) override;
};

}  // namespace cfdp

#endif  // FSFW_TESTS_CFDP_USERMOCK_H
