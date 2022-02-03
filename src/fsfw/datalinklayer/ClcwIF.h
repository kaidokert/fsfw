/**
 * @file	ClcwIF.h
 * @brief	This file defines the ClcwIF class.
 * @date	17.04.2013
 * @author	baetz
 */

#ifndef CLCWIF_H_
#define CLCWIF_H_

#include <stdint.h>

/**
 * Interface to manage a CLCW register.
 * @ingroup ccsds_handling
 */
class ClcwIF {
 public:
  /**
   * Empty virtual destructor.
   */
  virtual ~ClcwIF() {}
  /**
   * Simple setter.
   * @param setChannel The virtual channel id to set.
   */
  virtual void setVirtualChannel(uint8_t setChannel) = 0;
  /**
   * Simple setter.
   * @param lockout status of the flag.
   */
  virtual void setLockoutFlag(bool lockout) = 0;
  /**
   * Simple setter.
   * @param waitFlag status of the flag.
   */
  virtual void setWaitFlag(bool waitFlag) = 0;
  /**
   * Simple setter.
   * @param retransmitFlag status of the flag.
   */
  virtual void setRetransmitFlag(bool retransmitFlag) = 0;
  /**
   * Sets the farm B count.
   * @param count A full 8bit counter value can be passed. Only the last three bits are used.
   */
  virtual void setFarmBCount(uint8_t count) = 0;
  /**
   * Simple setter.
   * @param vR Value of vR.
   */
  virtual void setReceiverFrameSequenceNumber(uint8_t vR) = 0;
  /**
   * Returns the register as a full 32bit value.
   * @return The value.
   */
  virtual uint32_t getAsWhole() = 0;
  /**
   * Sets the whole content to this value.
   * @param rawClcw	The value to set the content.
   */
  virtual void setWhole(uint32_t rawClcw) = 0;
  /**
   * Debug method to print the CLCW.
   */
  virtual void print() = 0;
};

#endif /* CLCWIF_H_ */
