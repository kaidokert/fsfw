#ifndef FSFW_TIMEMANAGER_COUNTDOWN_H_
#define FSFW_TIMEMANAGER_COUNTDOWN_H_

#include "Clock.h"

/**
 *
 * Countdown keeps track of a timespan.
 *
 * Countdown::resetTimer restarts the timer.
 * Countdown::setTimeout sets a new countdown duration and resets.
 *
 * Can be checked with Countdown::hasTimedOut or
 * Countdown::isBusy.
 *
 * Countdown::timeOut will force the timer to time out.
 *
 */
class Countdown {
 public:
  /**
   * Constructor which sets the countdown duration in milliseconds
   *
   * It does not start the countdown!
   * Call resetTimer or setTimeout before usage!
   * Otherwise a call to hasTimedOut might return True.
   *
   * @param initialTimeout Countdown duration in milliseconds
   */
  Countdown(uint32_t initialTimeout = 0);
  ~Countdown();
  /**
   * Call to set a new countdown duration.
   *
   * Resets the countdown!
   *
   * @param milliseconds new countdown duration in milliseconds
   * @return Returnvalue from Clock::getUptime
   */
  ReturnValue_t setTimeout(uint32_t milliseconds);
  /**
   * Returns true if the countdown duration has passed.
   *
   * @return True if the countdown has passed
   * 		   False if it is still running
   */
  bool hasTimedOut() const;
  /**
   * Complementary to hasTimedOut.
   *
   * @return True if the countdown is till running
   * 		   False if it is still running
   */
  bool isBusy() const;
  /**
   *  Uses last set timeout value and restarts timer.
   */
  ReturnValue_t resetTimer();
  /**
   * Returns the remaining milliseconds (0 if timeout)
   */
  uint32_t getRemainingMillis() const;
  /**
   * Makes hasTimedOut() return true
   */
  void timeOut();
  /**
   * Internal countdown duration in milliseconds
   */
  uint32_t timeout;

 private:
  /**
   * Last time the timer was started (uptime)
   */
  uint32_t startTime = 0;

  uint32_t getCurrentTime() const;
};

#endif /* FSFW_TIMEMANAGER_COUNTDOWN_H_ */
