#include "RtemsBasic.h"


//ReturnValue_t RtemsBasic::convertReturnCode(rtems_status_code inValue) {
//	if (inValue == RTEMS_SUCCESSFUL) {
//		return HasReturnvaluesIF::RETURN_OK;
//	} else {
//		switch(inValue){
//		case RTEMS_SUCCESSFUL:
//			return OperatingSystemIF::SUCCESSFUL;
//		case RTEMS_TASK_EXITTED:
//			return OperatingSystemIF::TASK_EXITTED;
//		case RTEMS_MP_NOT_CONFIGURED:
//			return OperatingSystemIF::MP_NOT_CONFIGURED;
//		case RTEMS_INVALID_NAME:
//			return OperatingSystemIF::INVALID_NAME;
//		case  RTEMS_INVALID_ID:
//			return OperatingSystemIF::INVALID_ID;
//		case RTEMS_TOO_MANY:
//			return OperatingSystemIF::TOO_MANY;
//		case RTEMS_TIMEOUT:
//			return OperatingSystemIF::TIMEOUT;
//		case RTEMS_OBJECT_WAS_DELETED:
//			return OperatingSystemIF::OBJECT_WAS_DELETED;
//		case RTEMS_INVALID_SIZE:
//			return OperatingSystemIF::INVALID_SIZE;
//		case RTEMS_INVALID_ADDRESS:
//			return OperatingSystemIF::INVALID_ADDRESS;
//		case RTEMS_INVALID_NUMBER:
//			return OperatingSystemIF::INVALID_NUMBER;
//		case RTEMS_NOT_DEFINED:
//			return OperatingSystemIF::NOT_DEFINED;
//		case RTEMS_RESOURCE_IN_USE:
//			return OperatingSystemIF::RESOURCE_IN_USE;
//		//TODO RTEMS_UNSATISFIED is double mapped for FLP so it will only return Queue_empty and not unsatisfied
//		case RTEMS_UNSATISFIED:
//			return OperatingSystemIF::QUEUE_EMPTY;
//		case RTEMS_INCORRECT_STATE:
//			return OperatingSystemIF::INCORRECT_STATE;
//		case RTEMS_ALREADY_SUSPENDED:
//			return OperatingSystemIF::ALREADY_SUSPENDED;
//		case RTEMS_ILLEGAL_ON_SELF:
//			return OperatingSystemIF::ILLEGAL_ON_SELF;
//		case RTEMS_ILLEGAL_ON_REMOTE_OBJECT:
//			return  OperatingSystemIF::ILLEGAL_ON_REMOTE_OBJECT;
//		case RTEMS_CALLED_FROM_ISR:
//			return OperatingSystemIF::CALLED_FROM_ISR;
//		case RTEMS_INVALID_PRIORITY:
//			return OperatingSystemIF::INVALID_PRIORITY;
//		case RTEMS_INVALID_CLOCK:
//			return OperatingSystemIF::INVALID_CLOCK;
//		case RTEMS_INVALID_NODE:
//			return OperatingSystemIF::INVALID_NODE;
//		case RTEMS_NOT_CONFIGURED:
//			return OperatingSystemIF::NOT_CONFIGURED;
//		case RTEMS_NOT_OWNER_OF_RESOURCE:
//			return OperatingSystemIF::NOT_OWNER_OF_RESOURCE;
//		case RTEMS_NOT_IMPLEMENTED:
//			return OperatingSystemIF::NOT_IMPLEMENTED;
//		case RTEMS_INTERNAL_ERROR:
//			return OperatingSystemIF::INTERNAL_ERROR;
//		case RTEMS_NO_MEMORY:
//			return OperatingSystemIF::NO_MEMORY;
//		case RTEMS_IO_ERROR:
//			return OperatingSystemIF::IO_ERROR;
//		default:
//			return HasReturnvaluesIF::RETURN_FAILED;
//		}
//	}
//}
