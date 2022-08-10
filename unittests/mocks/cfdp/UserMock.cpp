#include "UserMock.h"

cfdp::UserMock::UserMock(HasFileSystemIF& vfs) : UserBase(vfs) {}

void cfdp::UserMock::transactionIndication(cfdp::TransactionId id) {}
void cfdp::UserMock::eofSentIndication(cfdp::TransactionId id) {}
void cfdp::UserMock::abandonedIndication(cfdp::TransactionId id, cfdp::ConditionCode code,
                                         uint64_t progress) {}
void cfdp::UserMock::eofRecvIndication(cfdp::TransactionId id) {}
void cfdp::UserMock::transactionFinishedIndication(TransactionFinishedParams finishedParams) {}
void cfdp::UserMock::metadataRecvdIndication(MetadataRecvdParams params) {}
void cfdp::UserMock::fileSegmentRecvdIndication(FileSegmentRecvdParams params) {}
void cfdp::UserMock::reportIndication(TransactionId id, StatusReportIF& report) {}
void cfdp::UserMock::suspendedIndication(TransactionId id, ConditionCode code) {}
void cfdp::UserMock::resumedIndication(TransactionId id, size_t progress) {}
void cfdp::UserMock::faultIndication(cfdp::TransactionId id, cfdp::ConditionCode code,
                                     size_t progress) {}
