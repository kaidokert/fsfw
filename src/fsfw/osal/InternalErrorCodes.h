#ifndef INTERNALERRORCODES_H_
#define INTERNALERRORCODES_H_

#include "fsfw/returnvalues/returnvalue.h"

class InternalErrorCodes {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::INTERNAL_ERROR_CODES;

  static const ReturnValue_t NO_CONFIGURATION_TABLE = MAKE_RETURN_CODE(0x01);
  static const ReturnValue_t NO_CPU_TABLE = MAKE_RETURN_CODE(0x02);
  static const ReturnValue_t INVALID_WORKSPACE_ADDRESS = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t TOO_LITTLE_WORKSPACE = MAKE_RETURN_CODE(0x04);
  static const ReturnValue_t WORKSPACE_ALLOCATION = MAKE_RETURN_CODE(0x05);
  static const ReturnValue_t INTERRUPT_STACK_TOO_SMALL = MAKE_RETURN_CODE(0x06);
  static const ReturnValue_t THREAD_EXITTED = MAKE_RETURN_CODE(0x07);
  static const ReturnValue_t INCONSISTENT_MP_INFORMATION = MAKE_RETURN_CODE(0x08);
  static const ReturnValue_t INVALID_NODE = MAKE_RETURN_CODE(0x09);
  static const ReturnValue_t NO_MPCI = MAKE_RETURN_CODE(0x0a);
  static const ReturnValue_t BAD_PACKET = MAKE_RETURN_CODE(0x0b);
  static const ReturnValue_t OUT_OF_PACKETS = MAKE_RETURN_CODE(0x0c);
  static const ReturnValue_t OUT_OF_GLOBAL_OBJECTS = MAKE_RETURN_CODE(0x0d);
  static const ReturnValue_t OUT_OF_PROXIES = MAKE_RETURN_CODE(0x0e);
  static const ReturnValue_t INVALID_GLOBAL_ID = MAKE_RETURN_CODE(0x0f);
  static const ReturnValue_t BAD_STACK_HOOK = MAKE_RETURN_CODE(0x10);
  static const ReturnValue_t BAD_ATTRIBUTES = MAKE_RETURN_CODE(0x11);
  static const ReturnValue_t IMPLEMENTATION_KEY_CREATE_INCONSISTENCY = MAKE_RETURN_CODE(0x12);
  static const ReturnValue_t IMPLEMENTATION_BLOCKING_OPERATION_CANCEL = MAKE_RETURN_CODE(0x13);
  static const ReturnValue_t MUTEX_OBTAIN_FROM_BAD_STATE = MAKE_RETURN_CODE(0x14);
  static const ReturnValue_t UNLIMITED_AND_MAXIMUM_IS_0 = MAKE_RETURN_CODE(0x15);

  virtual ~InternalErrorCodes();

  static ReturnValue_t translate(uint8_t code);

 private:
  InternalErrorCodes();
};

#endif /* INTERNALERRORCODES_H_ */
