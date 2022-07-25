#ifndef FSFW_TESTS_INTERNALERRORREPORTERMOCK_H
#define FSFW_TESTS_INTERNALERRORREPORTERMOCK_H

#include "fsfw/internalerror/InternalErrorReporterIF.h"

class InternalErrorReporterMock: public InternalErrorReporterIF {
 public:
  InternalErrorReporterMock();
 private:
};
#endif  // FSFW_TESTS_INTERNALERRORREPORTERMOCK_H
