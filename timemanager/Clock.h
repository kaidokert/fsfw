#ifndef FRAMEWORK_TIMEMANAGER_CLOCK_H_
#define FRAMEWORK_TIMEMANAGER_CLOCK_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../ipc/MutexHelper.h"
#include "../globalfunctions/timevalOperations.h"

#include <cstdint>
#include <sys/time.h>

//! Don't use these for time points, type is not large enough for UNIX epoch.
using dur_millis_t = uint32_t;

class Clock {
public:
	typedef struct {
	  uint32_t year; //!< Year, A.D.
	  uint32_t month; //!< Month, 1 .. 12.
	  uint32_t day; //!< Day, 1 .. 31.
	  uint32_t hour; //!< Hour, 0 .. 23.
	  uint32_t minute; //!< Minute, 0 .. 59.
	  uint32_t second; //!< Second, 0 .. 59.
	  uint32_t usecond; //!< Microseconds, 0 .. 999999
	}   TimeOfDay_t;

	/**
	 * This method returns the number of clock ticks per second.
	 * In RTEMS, this is typically 1000.
	 * @return	The number of ticks.
	 *
	 * @deprecated, we should not worry about ticks, but only time
	 */
	static uint32_t getTicksPerSecond(void);
	/**
	 * This system call sets the system time.
	 * To set the time, it uses a TimeOfDay_t struct.
	 * @param time The struct with the time settings to set.
	 * @return	-@c RETURN_OK on success. Otherwise, the OS failure code
	 * 				is returned.
	 */
	static ReturnValue_t setClock(const TimeOfDay_t* time);
	/**
	 * This system call sets the system time.
	 * To set the time, it uses a timeval struct.
	 * @param time The struct with the time settings to set.
	 * @return	-@c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setClock(const timeval* time);
	/**
	 * This system call returns the current system clock in timeval format.
	 * The timval format has the fields @c tv_sec with seconds and @c tv_usec with
	 * microseconds since an OS-defined epoch.
	 * @param time	A pointer to a timeval struct where the current time is stored.
	 * @return @c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getClock_timeval(timeval* time);

	/**
	 * Get the time since boot in a timeval struct
	 *
	 * @param[out] time A pointer to a timeval struct where the uptime is stored.
	 * @return @c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 *
	 * @deprecated, I do not think this should be able to fail, use timeval getUptime()
	 */
	static ReturnValue_t getUptime(timeval* uptime);

	static timeval getUptime();

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
	static ReturnValue_t convertTimeOfDayToTimeval(const TimeOfDay_t* from,
			timeval* to);

	/**
	 * Converts a time represented as seconds and subseconds since unix epoch to days since J2000
	 *
	 * @param time seconds since unix epoch
	 * @param[out] JD2000 days since J2000
	 * @return \c RETURN_OK
	 */
	static ReturnValue_t convertTimevalToJD2000(timeval time, double* JD2000);

	/**
	 * Calculates and adds the offset between UTC and TT
	 *
	 * Depends on the leap seconds to be set correctly.
	 *
	 * @param utc timeval, corresponding to UTC time
	 * @param[out] tt timeval, corresponding to Terrestial Time
	 * @return \c RETURN_OK on success, \c RETURN_FAILED if leapSeconds are not set
	 */
	static ReturnValue_t convertUTCToTT(timeval utc, timeval* tt);

	/**
	 * Set the Leap Seconds since 1972
	 *
	 * @param leapSeconds_
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t setLeapSeconds(const uint16_t leapSeconds_);

	/**
	 * Get the Leap Seconds since 1972
	 *
	 * Must be set before!
	 *
	 * @param[out] leapSeconds_
	 * @return \c RETURN_OK on success. Otherwise, the OS failure code is returned.
	 */
	static ReturnValue_t getLeapSeconds(uint16_t *leapSeconds_);

	/**
	 * Function to check and create the Mutex for the clock
	 * @return \c RETURN_OK on success. Otherwise \c RETURN_FAILED if not able to create one
	 */
	static ReturnValue_t checkOrCreateClockMutex();

private:
	static MutexIF* timeMutex;
	static uint16_t leapSeconds;
};


#endif /* FRAMEWORK_TIMEMANAGER_CLOCK_H_ */
