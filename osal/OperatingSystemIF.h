#ifndef FRAMEWORK_OSAL_OPERATINGSYSTEMIF_H_
#define FRAMEWORK_OSAL_OPERATINGSYSTEMIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

class OperatingSystemIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::OPERATING_SYSTEM_ABSTRACTION;

	//API Status codes, must be implemented by the Operating System (TODO comments based on rtems status.h):

	/**
	 *  This is the status to indicate successful completion.
	*/
	static const ReturnValue_t SUCCESSFUL = MAKE_RETURN_CODE(0);
	  /**
	   *  This is the status to indicate that a thread exited.
	   */
	static const ReturnValue_t TASK_EXITTED = MAKE_RETURN_CODE(1);
	  /**
	   *  This is the status to indicate multiprocessing is not configured.
	   */
	static const ReturnValue_t MP_NOT_CONFIGURED = MAKE_RETURN_CODE(2);
	  /**
	   *  This is the status to indicate that the object name was invalid.
	   */
	static const ReturnValue_t INVALID_NAME = MAKE_RETURN_CODE(3);
	  /**
	   *  This is the status to indicate that the object Id was invalid.
	   */
	static const ReturnValue_t INVALID_ID = MAKE_RETURN_CODE(4);
	  /**
	   *  This is the status to indicate you have attempted to create too many
	   *  instances of a particular object class.
	   *
	   *  Used for full messages Queues as well
	   */
	static const ReturnValue_t TOO_MANY = MAKE_RETURN_CODE(5);
	  /**
	   *  This is the status to indicate that a blocking directive timed out.
	   */
	static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(6);
	  /**
	   *  This is the status to indicate the the object was deleted
	   *  while the task was blocked waiting.
	   */
	static const ReturnValue_t OBJECT_WAS_DELETED = MAKE_RETURN_CODE(7);
	  /**
	   *  This is the status to indicate that the specified size was invalid.
	   */
	static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(8);
	  /**
	   *  This is the status to indicate that the specified address is invalid.
	   */
	static const ReturnValue_t INVALID_ADDRESS = MAKE_RETURN_CODE(9);
	  /**
	   *  This is the status to indicate that the specified number was invalid.
	   */
	static const ReturnValue_t INVALID_NUMBER = MAKE_RETURN_CODE(10);
	  /**
	   *  This is the status to indicate that the item has not been initialized.
	   */
	static const ReturnValue_t NOT_DEFINED =MAKE_RETURN_CODE(11);
	  /**
	   *  This is the status to indicate that the object still has
	   *  resources in use.
	   */
	static const ReturnValue_t RESOURCE_IN_USE =MAKE_RETURN_CODE(12);
	  /**
	   *  This is the status to indicate that the request was not satisfied.
	   */
	static const ReturnValue_t UNSATISFIED =MAKE_RETURN_CODE(13);
	/**
	 * Indicates that a Message Queue is empty (unable to allocate it)
	 */
	static const ReturnValue_t QUEUE_EMPTY =MAKE_RETURN_CODE(14);
	  /**
	   *  This is the status to indicate that a thread is in wrong state
	   *  was in the wrong execution state for the requested operation.
	   */
	static const ReturnValue_t INCORRECT_STATE =MAKE_RETURN_CODE(15);
	  /**
	   *  This is the status to indicate thread was already suspended.
	   */
	static const ReturnValue_t ALREADY_SUSPENDED = MAKE_RETURN_CODE(16);
	  /**
	   *  This is the status to indicate that the operation is illegal
	   *  on calling thread.
	   */
	static const ReturnValue_t ILLEGAL_ON_SELF =MAKE_RETURN_CODE(17);
	  /**
	   *  This is the status to indicate illegal for remote object.
	   */
	static const ReturnValue_t ILLEGAL_ON_REMOTE_OBJECT=MAKE_RETURN_CODE(18);
	  /**
	   *  This is the status to indicate that the operation should not be
	   *  called from from this excecution environment.
	   */
	static const ReturnValue_t CALLED_FROM_ISR=MAKE_RETURN_CODE(19);
	  /**
	   *  This is the status to indicate that an invalid thread priority
	   *  was provided.
	   */
	static const ReturnValue_t INVALID_PRIORITY=MAKE_RETURN_CODE(20);
	  /**
	   *  This is the status to indicate that the specified date/time was invalid.
	   */
	static const ReturnValue_t INVALID_CLOCK=MAKE_RETURN_CODE(21);
	  /**
	   *  This is the status to indicate that the specified node Id was invalid.
	   */
	static const ReturnValue_t INVALID_NODE=MAKE_RETURN_CODE(22);
	  /**
	   *  This is the status to indicate that the directive was not configured.
	   */
	static const ReturnValue_t NOT_CONFIGURED=MAKE_RETURN_CODE(23);
	  /**
	   *  This is the status to indicate that the caller is not the
	   *  owner of the resource.
	   */
	static const ReturnValue_t NOT_OWNER_OF_RESOURCE=MAKE_RETURN_CODE(24);
	  /**
	   *  This is the status to indicate the the directive or requested
	   *  portion of the directive is not implemented.
	   */
	static const ReturnValue_t NOT_IMPLEMENTED=MAKE_RETURN_CODE(25);
	  /**
	   *  This is the status to indicate that an internal RTEMS inconsistency
	   *  was detected.
	   */
	static const ReturnValue_t INTERNAL_ERROR=MAKE_RETURN_CODE(26);
	  /**
	   *  This is the status to indicate that the directive attempted to allocate
	   *  memory but was unable to do so.
	   */
	static const ReturnValue_t NO_MEMORY=MAKE_RETURN_CODE(27);
	  /**
	   *  This is the status to indicate an driver IO error.
	   */
	static const ReturnValue_t IO_ERROR=MAKE_RETURN_CODE(28);

	virtual ~OperatingSystemIF() {};
};
#endif /* FRAMEWORK_OSAL_OPERATINGSYSTEMIF_H_ */
