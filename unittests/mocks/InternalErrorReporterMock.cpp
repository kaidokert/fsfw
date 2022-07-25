#include "InternalErrorReporterMock.h"

InternalErrorReporterMock::InternalErrorReporterMock() = default;

void InternalErrorReporterMock::queueMessageNotSent() { queueMsgNotSentCallCnt++; }
void InternalErrorReporterMock::lostTm() { lostTmCallCnt++; }
void InternalErrorReporterMock::storeFull() { storeFullCallCnt++; }
