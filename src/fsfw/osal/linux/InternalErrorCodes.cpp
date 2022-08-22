#include "fsfw/osal/InternalErrorCodes.h"

ReturnValue_t InternalErrorCodes::translate(uint8_t code) {
  // TODO This class can be removed
  return returnvalue::FAILED;
}

InternalErrorCodes::InternalErrorCodes() {}

InternalErrorCodes::~InternalErrorCodes() {}
