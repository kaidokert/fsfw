#ifndef FSFW_TESTS_CFDP_USERMOCK_H
#define FSFW_TESTS_CFDP_USERMOCK_H

#include <queue>

#include "fsfw/cfdp/handler/UserBase.h"

namespace cfdp {

class UserMock : public UserBase {
 public:
  explicit UserMock(HasFileSystemIF& vfs);

  void transactionIndication(const TransactionId& id) override;
  void eofSentIndication(const TransactionId& id) override;
  void abandonedIndication(const TransactionId& id, ConditionCode code, size_t progress) override;
  void eofRecvIndication(const TransactionId& id) override;
  void transactionFinishedIndication(const TransactionFinishedParams& params) override;
  void metadataRecvdIndication(const MetadataRecvdParams& params) override;
  void fileSegmentRecvdIndication(const FileSegmentRecvdParams& params) override;
  void reportIndication(const TransactionId& id, StatusReportIF& report) override;
  void suspendedIndication(const TransactionId& id, ConditionCode code) override;
  void resumedIndication(const TransactionId& id, size_t progress) override;
  void faultIndication(const TransactionId& id, ConditionCode code, size_t progress) override;

  std::queue<std::pair<TransactionId, MetadataRecvdParams>> metadataRecvd;
  std::queue<TransactionId> eofsRevd;
  std::queue<std::pair<TransactionId, TransactionFinishedParams>> finishedRecvd;
  void reset();
};

}  // namespace cfdp

#endif  // FSFW_TESTS_CFDP_USERMOCK_H
