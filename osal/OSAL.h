/**
 * @file	OSAL.h
 * @brief	This file defines the OSAL class.
 * @date	19.12.2012
 * @author	baetz
 */

#ifndef OSAL_H_
#define OSAL_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

#define RTEMS_API 1

#ifndef API
#error Please specify Operating System API. Supported: API=RTEMS_API
#elif API == RTEMS_API
#include <rtems/endian.h>
#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/error.h>
#include <rtems/stackchk.h>
#include <stdint.h>

//include RMAP

#define opus_main 				Init2
#define opus_main_arguments		rtems_task_argument

//Typedefs for RTEMS:	//TODO: these are global, shouldn't they have some prefix?
/**
 * A typedef to pass options to RTEMS elements.
 */
typedef rtems_option Option_t;
/**
 * A typedef to pass attributes to RTEMS elements.
 */
typedef rtems_attribute Attribute_t;
/**
 * A typedef for RTEMS task modes.
 */
typedef rtems_mode OpusMode_t;
/**
 * A typedef for time intervals. In clock ticks.
 */
typedef rtems_interval Interval_t;
typedef rtems_time_of_day TimeOfDay_t;
typedef rtems_id TaskId_t;
typedef rtems_id PeriodId_t;
typedef rtems_id MutexId_t;
typedef rtems_id MessageQueueId_t;
typedef rtems_name Name_t;

typedef rtems_task_argument TaskArgument_t;
typedef rtems_task_entry TaskEntry_t;
typedef rtems_task TaskReturn_t;
typedef rtems_task_priority TaskPriority_t;
typedef rtems_isr_entry IsrHandler_t;
typedef rtems_isr IsrReturn_t;
typedef rtems_vector_number InterruptNumber_t;

/**
 * This class contains encapsulates all System Calls to the Operating System.
 * It is currently tailored to the RTEMS Operating system, but should in general
 * support different OS.
 * For more detailed information on the Operating System calls, please refer to the
 * RTEMS documentation at www.rtems.org .
 */
class OSAL: public HasReturnvaluesIF {
private:
	/**
	 * A method to convert an OS-specific return code to the frameworks return value concept.
	 * @param inValue The return code coming from the OS.
	 * @return The converted return value.
	 */
	static ReturnValue_t convertReturnCode(uint8_t inValue);
public:

	static const uint8_t INTERFACE_ID = OPERATING_SYSTEM_ABSTRACTION;
	//Interrupts:
	static const uint32_t INTERRUPT_MASK_REGISTER_ADDRESS = 0x80000240;
	//API Status codes:
	static const ReturnValue_t SUCCESSFUL = RTEMS_SUCCESSFUL;
	static const ReturnValue_t TASK_EXITTED =
			MAKE_RETURN_CODE( RTEMS_TASK_EXITTED );
	static const ReturnValue_t MP_NOT_CONFIGURED =
			MAKE_RETURN_CODE( RTEMS_MP_NOT_CONFIGURED );
	static const ReturnValue_t INVALID_NAME =
			MAKE_RETURN_CODE( RTEMS_INVALID_NAME );
	static const ReturnValue_t INVALID_ID = MAKE_RETURN_CODE( RTEMS_INVALID_ID );
	static const ReturnValue_t TOO_MANY = MAKE_RETURN_CODE( RTEMS_TOO_MANY );
	static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE( RTEMS_TIMEOUT );
	static const ReturnValue_t OBJECT_WAS_DELETED =
			MAKE_RETURN_CODE( RTEMS_OBJECT_WAS_DELETED );
	static const ReturnValue_t INVALID_SIZE =
			MAKE_RETURN_CODE( RTEMS_INVALID_SIZE );
	static const ReturnValue_t INVALID_ADDRESS =
			MAKE_RETURN_CODE( RTEMS_INVALID_ADDRESS );
	static const ReturnValue_t INVALID_NUMBER =
			MAKE_RETURN_CODE( RTEMS_INVALID_NUMBER );
	static const ReturnValue_t NOT_DEFINED =
			MAKE_RETURN_CODE( RTEMS_NOT_DEFINED );
	static const ReturnValue_t RESOURCE_IN_USE =
			MAKE_RETURN_CODE( RTEMS_RESOURCE_IN_USE );
	static const ReturnValue_t UNSATISFIED =
			MAKE_RETURN_CODE( RTEMS_UNSATISFIED );
	static const ReturnValue_t QUEUE_EMPTY =
			MAKE_RETURN_CODE( RTEMS_UNSATISFIED );
	static const ReturnValue_t INCORRECT_STATE =
			MAKE_RETURN_CODE( RTEMS_INCORRECT_STATE );
	static const ReturnValue_t ALREADY_SUSPENDED =
			MAKE_RETURN_CODE( RTEMS_ALREADY_SUSPENDED );
	static const ReturnValue_t ILLEGAL_ON_SELF =
			MAKE_RETURN_CODE( RTEMS_ILLEGAL_ON_SELF );
	static const ReturnValue_t ILLEGAL_ON_REMOTE_OBJECT =
			MAKE_RETURN_CODE( RTEMS_ILLEGAL_ON_REMOTE_OBJECT );
	static const ReturnValue_t CALLED_FROM_ISR =
			MAKE_RETURN_CODE( RTEMS_CALLED_FROM_ISR );
	static const ReturnValue_t INVALID_PRIORITY =
			MAKE_RETURN_CODE( RTEMS_INVALID_PRIORITY );
	static const ReturnValue_t INVALID_CLOCK =
			MAKE_RETURN_CODE( RTEMS_INVALID_CLOCK );
	static const ReturnValue_t INVALID_NODE =
			MAKE_RETURN_CODE( RTEMS_INVALID_NODE );
	static const ReturnValue_t NOT_CONFIGURED =
			MAKE_RETURN_CODE( RTEMS_NOT_CONFIGURED );
	static const ReturnValue_t NOT_OWNER_OF_RESOURCE =
			MAKE_RETURN_CODE( RTEMS_NOT_OWNER_OF_RESOURCE );
	static const ReturnValue_t NOT_IMPLEMENTED =
			MAKE_RETURN_CODE( RTEMS_NOT_IMPLEMENTED );
	static const ReturnValue_t INTERNAL_ERROR =
			MAKE_RETURN_CODE( RTEMS_INTERNAL_ERROR );
	static const ReturnValue_t NO_MEMORY = MAKE_RETURN_CODE( RTEMS_NO_MEMORY );
	static const ReturnValue_t IO_ERROR = MAKE_RETURN_CODE( RTEMS_IO_ERROR );
	//API options:
	static const Option_t DEFAULT_OPTIONS = RTEMS_DEFAULT_OPTIONS;
	static const Option_t WAIT = RTEMS_WAIT;
	static const Option_t NO_WAIT = RTEMS_NO_WAIT;
	static const Option_t EVENT_ALL = RTEMS_EVENT_ALL;
	static const Option_t EVENT_ANY = RTEMS_EVENT_ANY;
	//API Attributes:
	static const Attribute_t DEFAULT_ATTRIBUTES = RTEMS_DEFAULT_ATTRIBUTES;
	static const Attribute_t LOCAL = RTEMS_LOCAL;
	static const Attribute_t GLOBAL = RTEMS_GLOBAL;
	static const Attribute_t FIFO = RTEMS_FIFO;
	static const Attribute_t PRIORITY = RTEMS_PRIORITY;
	static const Attribute_t NO_FLOATING_POINT = RTEMS_NO_FLOATING_POINT;
	static const Attribute_t FLOATING_POINT = RTEMS_FLOATING_POINT;
	//API Modes:
	static const OpusMode_t ALL_MODE_MASKS = RTEMS_ALL_MODE_MASKS;
	static const OpusMode_t DEFAULT_MODES = RTEMS_DEFAULT_MODES;
	static const OpusMode_t CURRENT_MODE = RTEMS_CURRENT_MODE;
	static const OpusMode_t PREEMPT = RTEMS_PREEMPT;
	static const OpusMode_t NO_PREEMPT = RTEMS_NO_PREEMPT;
	static const OpusMode_t NO_TIMESLICE = RTEMS_NO_TIMESLICE;
	static const OpusMode_t TIMESLICE = RTEMS_TIMESLICE;
	static const OpusMode_t ASR = RTEMS_ASR;
	static const OpusMode_t NO_ASR = RTEMS_NO_ASR;
	//API Time and Timing
	static const Interval_t MILISECOND_WAIT = 1;
	static const Interval_t NO_TIMEOUT = RTEMS_NO_TIMEOUT;
	static const TaskId_t TASK_MYSELF = RTEMS_SELF;
	static const size_t MINIMUM_STACK_SIZE = RTEMS_MINIMUM_STACK_SIZE;
	/**
	 * This is a helper method to build a qualified name out of single characters
	 * @param c1	The first character
	 * @param c2	The second character
	 * @param c3	The third character
	 * @param c4	The fourth character
	 * @return	A name suitable for use for the Operating System
	 */
	static Name_t buildName(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
	/**
	 * This method returns the number of clock ticks per second.
	 * In RTEMS, this is typically 1000.
	 * @return	The number of ticks.
	 */
	static Interval_t getTicksPerSecond(void);
	/**
	 * This system call sets the system time.
	 * To set the time, it uses a TimeOfDay_t struct.
	 * @param time The struct with the time settings to set.
	 * @return	\c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setClock(TimeOfDay_t* time);
	/**
	 * This system call sets the system time.
	 * To set the time, it uses a timeval struct.
	 * @param time The struct with the time settings to set.
	 * @return	\c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setClock(timeval* time);
	/**
	 * This system call returns the current system clock in timeval format.
	 * The timval format has the fields \c tv_sec with seconds and \c tv_usec with
	 * microseconds since an OS-defined epoch.
	 * @param time	A pointer to a timeval struct where the current time is stored.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getClock_timeval(timeval* time);

	/**
	 * Get the time since boot in a timeval struct
	 *
	 * @param[out] time A pointer to a timeval struct where the uptime is stored.
	 * @return\c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getUptime(timeval* uptime);

	/**
	 * Get the time since boot in milliseconds
	 *
	 * This value can overflow! Still, it can be used to calculate time intervalls
	 * between two calls up to 49 days by always using uint32_t in the calculation
	 *
	 * @param ms uptime in ms
	 * @return RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getUptime(uint32_t* uptimeMs);

	/**
	 * Returns the time in microseconds since an OS-defined epoch.
	 * The time is returned in a 64 bit unsigned integer.
	 * @param time A pointer to a 64 bit unisigned integer where the data is stored.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getClock_usecs(uint64_t* time);
	/**
	 * Returns the time in a TimeOfDay_t struct.
	 * @param time A pointer to a TimeOfDay_t struct.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getDateAndTime(TimeOfDay_t* time);
	/**
	 * Converts a time of day struct to POSIX seconds.
	 * @param time The time of day as input
	 * @param timeval The corresponding seconds since the epoch.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t convertTimeOfDayToTimeval(const TimeOfDay_t* from, timeval* to);
	/**
	 * Commands the calling task to sleep for a certain number of clock ticks.
	 * Typically other tasks are executed then.
	 * @param ticks	The number of clock ticks to sleep.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t sleepFor(Interval_t ticks);

	/**
	 * With this call, a new task is created.
	 * The task is created (its resources are acquired), but it is not started.
	 * @param name	A name as specified in the OS.
	 * @param initial_priority	The task's priority. Ranging from 0 (lowest) to 99 (highest).
	 * @param stack_size		The stack size reserved for this task.
	 * @param initial_modes		Options for the task's mode (preemptible, time slicing, etc..)
	 * @param attribute_set		Options for the task's attributes (floating point, local/global)
	 * @param id				The OS returns the task id, that uniquely identifies the task.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t createTask(Name_t name,
			TaskPriority_t initial_priority, size_t stack_size,
			OpusMode_t initial_modes, Attribute_t attribute_set, TaskId_t *id);
	/**
	 * Finds a task with the help of its name.
	 * @param name	Name of the task to find
	 * @param id	The OS returns the task id, that uniquely identifies the task.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t findTask(Name_t name, TaskId_t *id);
	/**
	 * Starts a task.
	 * The task immediately starts running.
	 * @param id The task id.
	 * @param entry_point	A pointer to the (static) method that is executed by the task.
	 * @param argument	One argument (a void* pointer) may be passed to the task.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t startTask(TaskId_t *id, TaskEntry_t entry_point,
			TaskArgument_t argument);
	/**
	 * With this call, tasks are deleted from the system
	 * @param id	The task id.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t deleteTask(TaskId_t *id);
	/**
	 * Checks if the current executing context is an ISR.
	 * @return true if handling an interrupt, false else.
	 */
	static bool isInterruptInProgress();
	/**
	 * An task is not executed periodically by default.
	 * This is activated with this call. This is managed internally.
	 * @param period		The task's period in clock ticks.
	 * @param[out] periodId	The newly created period's id
	 * @param name			optional name for the period
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setAndStartPeriod(Interval_t period, PeriodId_t *periodId, Name_t name = (('P' << 24) + ('e' << 16) + ('r' << 8) + 'd'));
	/**
	 * This call must be made in a periodic task, when activities of one cycle are finished.
	 * This is managed internally.
	 * @param periodId	Id of the period as returned by setAndStartPeriod()
	 * @param period	The period duration for the next cycle.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t checkAndRestartPeriod(PeriodId_t periodId, Interval_t period);
	/**
	 * This call deletes the period.
	 * This is managed internally.
	 * @param id	Pointer to the task identifier the period belongs to.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t deletePeriod(TaskId_t *id);
	/**
	 * With this call the period statistics (and therefore the periodic task
	 * statistics) are printed to the screen.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t reportPeriodStatistics();

	/**
	 * With this call, a new message queue is created.
	 * @param name		A qualified name for the message queue.
	 * @param count		Number of messages the queue can store before it rejects new messages.
	 * @param max_message_size	Maximum size of a single message.
	 * @param attribute_set		Attributes for the message queue (fifo/priority, local/global)
	 * @param id		A unique message queue identifier returned by the OS.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t createMessageQueue(Name_t name, uint32_t count,
			size_t max_message_size, Attribute_t attribute_set,
			MessageQueueId_t *id);
	/**
	 * Returns a message queue id by its name.
	 * @param name	The queue's name.
	 * @param id	A pointer to the queue id to return to.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t findMessageQueue(Name_t name, MessageQueueId_t *id);
	/**
	 * Sends a message to the queue given by id.
	 * @param id		Id of the queue to send to
	 * @param buffer	A pointer to any kind of data to send over the queue.
	 * @param size		Size of the data to send.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t sendMessage(MessageQueueId_t id, const void *buffer,
			size_t size);
	/**
	 * Checks, if a message was received by a queue with identifier id.
	 * @param id		The id of the checked task.
	 * @param buffer	Pointer to the buffer to store to.
	 * @param bufSize	Maximum size of the buffer.
	 * @param recSize	The actual message size is returned here.
	 * @param option_set	Specifies, if the task waits for a message (WAIT/ NO_WAIT).
	 * @param timeout	If the task waits, this interval specifies how long (in clock ticks).
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t receiveMessage(MessageQueueId_t id, void *buffer,
			size_t bufSize, size_t *recSize, Option_t option_set,
			Interval_t timeout);
	/**
	 * Deletes all pending messages in a certain queue.
	 * @param id	Id of the queue to flush
	 * @param count	Number of flushed messages.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t flushMessageQueue( MessageQueueId_t id, uint32_t* count );
	/**
	 * Deletes a message queue from the system.
	 * @param id	Id of the queue to delete.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t deleteMessageQueue(MessageQueueId_t *id);

	/**
	 * Creates a new mutual exclusive lock (or semaphore).
	 * With these locks, concurrent access to system resources (data pool, ...) can be
	 * controlled.
	 * @param name	A qualified name for the mutex.
	 * @param id	The mutex's id as returned by the OS.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t createMutex(Name_t name, MutexId_t *id);
	/**
	 * Deletes the mutex identified by id.
	 * @param id Id of the mutex to delete.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t deleteMutex(MutexId_t *id);
	/**
	 * With this call, a task tries to acquire the mutex.
	 * Must be used in conjunction with unlockMutex.
	 * @param id		Id of the mutex to acquire.
	 * @param timeout	Specifies how long a task waits for the mutex. Default is NO_TIMEOUT.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t lockMutex(MutexId_t *id, Interval_t timeout);
	/**
	 * Releases a mutex.
	 * Must be used in conjunction with lockMutex.
	 * @param id	Id of the mutex to release.
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t unlockMutex(MutexId_t *id);
	/**
	 * Establishes a new interrupt service routine.
	 * @param handler	The service routine to establish
	 * @param interrupt	The interrupt (NOT trap type) the routine shall react to.
	 * @return	RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setInterruptServiceRoutine(IsrHandler_t handler, InterruptNumber_t interrupt, IsrHandler_t *oldHandler = NULL );
	/**
	 * Enables the interrupt given.
	 * The function tests, if the InterruptMask register was written successfully.
	 * @param interrupt The interrupt to enable.
	 * @return RETURN_OK if the interrupt was set successfully. RETURN_FAILED else.
	 */
	static ReturnValue_t enableInterrupt( InterruptNumber_t interrupt );
	/**
	 * Disables the interrupt given.
	 * @param interrupt The interrupt to disable.
	 * @return RETURN_OK if the interrupt was set successfully. RETURN_FAILED else.
	 */
	static ReturnValue_t disableInterrupt( InterruptNumber_t interrupt );
};

#endif /* API */
#endif /* OSAL_H_ */
