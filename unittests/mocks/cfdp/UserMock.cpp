#include "UserMock.h"

namespace cfdp {

cfdp::UserMock::UserMock(HasFileSystemIF& vfs) : UserBase(vfs) {}

void UserMock::transactionIndication(const TransactionId& id) {}
void UserMock::eofSentIndication(const TransactionId& id) {}
void UserMock::abandonedIndication(const TransactionId& id, cfdp::ConditionCode code,
                                   uint64_t progress) {}

void UserMock::eofRecvIndication(const TransactionId& id) { eofsRevd.push(id); }

void UserMock::transactionFinishedIndication(const TransactionFinishedParams& finishedParams) {
  finishedRecvd.push({finishedParams.id, finishedParams});
}

void UserMock::metadataRecvdIndication(const MetadataRecvdParams& params) {
  metadataRecvd.push({params.id, params});
}

void UserMock::fileSegmentRecvdIndication(const FileSegmentRecvdParams& params) {}
void UserMock::reportIndication(const TransactionId& id, StatusReportIF& report) {}
void UserMock::suspendedIndication(const TransactionId& id, ConditionCode code) {}
void UserMock::resumedIndication(const TransactionId& id, size_t progress) {}
void UserMock::faultIndication(const TransactionId& id, cfdp::ConditionCode code, size_t progress) {
}

void UserMock::reset() {
  std::queue<TransactionId>().swap(eofsRevd);
  std::queue<std::pair<TransactionId, cfdp::MetadataRecvdParams>>().swap(metadataRecvd);
  std::queue<std::pair<TransactionId, cfdp::TransactionFinishedParams>>().swap(finishedRecvd);
}

}  // namespace cfdp
