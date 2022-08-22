#include "fsfw/osal/InternalErrorCodes.h"

#include <rtems/score/interr.h>

ReturnValue_t InternalErrorCodes::translate(uint8_t code) {
  switch (code) {
      // TODO It looks like RTEMS-5 does not provide the same error codes
      //	case INTERNAL_ERROR_NO_CONFIGURATION_TABLE:
      //		return NO_CONFIGURATION_TABLE;
      //	case INTERNAL_ERROR_NO_CPU_TABLE:
      //		return NO_CPU_TABLE;
      //	case INTERNAL_ERROR_INVALID_WORKSPACE_ADDRESS:
      //		return INVALID_WORKSPACE_ADDRESS;
    case INTERNAL_ERROR_TOO_LITTLE_WORKSPACE:
      return TOO_LITTLE_WORKSPACE;
      //	case INTERNAL_ERROR_WORKSPACE_ALLOCATION:
      //		return WORKSPACE_ALLOCATION;
      //	case INTERNAL_ERROR_INTERRUPT_STACK_TOO_SMALL:
      //		return INTERRUPT_STACK_TOO_SMALL;
    case INTERNAL_ERROR_THREAD_EXITTED:
      return THREAD_EXITTED;
    case INTERNAL_ERROR_INCONSISTENT_MP_INFORMATION:
      return INCONSISTENT_MP_INFORMATION;
    case INTERNAL_ERROR_INVALID_NODE:
      return INVALID_NODE;
    case INTERNAL_ERROR_NO_MPCI:
      return NO_MPCI;
    case INTERNAL_ERROR_BAD_PACKET:
      return BAD_PACKET;
    case INTERNAL_ERROR_OUT_OF_PACKETS:
      return OUT_OF_PACKETS;
    case INTERNAL_ERROR_OUT_OF_GLOBAL_OBJECTS:
      return OUT_OF_GLOBAL_OBJECTS;
    case INTERNAL_ERROR_OUT_OF_PROXIES:
      return OUT_OF_PROXIES;
    case INTERNAL_ERROR_INVALID_GLOBAL_ID:
      return INVALID_GLOBAL_ID;
#ifndef STM32H743ZI_NUCLEO
    case INTERNAL_ERROR_BAD_STACK_HOOK:
      return BAD_STACK_HOOK;
#endif
      //	case INTERNAL_ERROR_BAD_ATTRIBUTES:
      //		return BAD_ATTRIBUTES;
      //	case INTERNAL_ERROR_IMPLEMENTATION_KEY_CREATE_INCONSISTENCY:
      //		return IMPLEMENTATION_KEY_CREATE_INCONSISTENCY;
      //	case INTERNAL_ERROR_IMPLEMENTATION_BLOCKING_OPERATION_CANCEL:
      //		return IMPLEMENTATION_BLOCKING_OPERATION_CANCEL;
      //	case INTERNAL_ERROR_MUTEX_OBTAIN_FROM_BAD_STATE:
      //		return MUTEX_OBTAIN_FROM_BAD_STATE;
      //	case INTERNAL_ERROR_UNLIMITED_AND_MAXIMUM_IS_0:
      //		return UNLIMITED_AND_MAXIMUM_IS_0;
    default:
      return returnvalue::FAILED;
  }
}

InternalErrorCodes::InternalErrorCodes() {}

InternalErrorCodes::~InternalErrorCodes() {}
