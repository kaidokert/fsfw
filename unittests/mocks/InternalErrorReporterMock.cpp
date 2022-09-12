#include "InternalErrorReporterMock.h"

InternalErrorReporterMock::InternalErrorReporterMock() = default;

void InternalErrorReporterMock::queueMessageNotSent() { queueMsgNotSentCallCnt++; }
void InternalErrorReporterMock::lostTm() { lostTmCallCnt++; }
void InternalErrorReporterMock::storeFull() { storeFullCallCnt++; }

void InternalErrorReporterMock::reset() {
  queueMsgNotSentCallCnt = 0;
  lostTmCallCnt = 0;
  storeFullCallCnt = 0;
}
