#include "UserMock.h"

namespace cfdp {

cfdp::UserMock::UserMock(HasFileSystemIF& vfs) : UserBase(vfs) {}

void UserMock::transactionIndication(const TransactionId& id) {}
void UserMock::eofSentIndication(const TransactionId& id) {}
void UserMock::abandonedIndication(const TransactionId& id, cfdp::ConditionCode code,
                                   uint64_t progress) {}
void UserMock::eofRecvIndication(const TransactionId& id) {}
void UserMock::transactionFinishedIndication(const TransactionFinishedParams& finishedParams) {}

void UserMock::metadataRecvdIndication(const MetadataRecvdParams& params) {
  MetadataRecvdParams copy = params;
  metadataRecvd.push(copy);
}

void UserMock::fileSegmentRecvdIndication(const FileSegmentRecvdParams& params) {}
void UserMock::reportIndication(const TransactionId& id, StatusReportIF& report) {}
void UserMock::suspendedIndication(const TransactionId& id, ConditionCode code) {}
void UserMock::resumedIndication(const TransactionId& id, size_t progress) {}
void UserMock::faultIndication(const TransactionId& id, cfdp::ConditionCode code, size_t progress) {
}

void UserMock::reset() {
  auto empty = std::queue<cfdp::MetadataRecvdParams>();
  metadataRecvd.swap(empty);
}

}  // namespace cfdp
