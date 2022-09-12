#ifndef FSFW_TESTS_INTERNALERRORREPORTERMOCK_H
#define FSFW_TESTS_INTERNALERRORREPORTERMOCK_H

#include "fsfw/internalerror/InternalErrorReporterIF.h"

class InternalErrorReporterMock : public InternalErrorReporterIF {
 public:
  unsigned int queueMsgNotSentCallCnt = 0;
  unsigned int lostTmCallCnt = 0;
  unsigned int storeFullCallCnt = 0;
  InternalErrorReporterMock();
  void reset();

 private:
  void queueMessageNotSent() override;
  void lostTm() override;
  void storeFull() override;
};
#endif  // FSFW_TESTS_INTERNALERRORREPORTERMOCK_H
