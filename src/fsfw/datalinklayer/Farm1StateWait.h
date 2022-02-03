/**
 * @file	Farm1StateWait.h
 * @brief	This file defines the Farm1StateWait class.
 * @date	24.04.2013
 * @author	baetz
 */

#ifndef FARM1STATEWAIT_H_
#define FARM1STATEWAIT_H_

#include "Farm1StateIF.h"
#include "dllConf.h"

/**
 * This class represents the FARM-1 "Wait" State.
 * The Wait state is reached if higher level procedures inform the FARM-1 Machine to wait
 * for a certain period. Currently, it is not in use.
 */
class Farm1StateWait : public Farm1StateIF {
 private:
  /**
   * This is a reference to the "owner" class the State works on.
   */
  VirtualChannelReception* myVC;

 public:
  /**
   * The default constructor if the State.
   * Sets the "owner" of the State.
   * @param setMyVC	The "owner" class.
   */
  Farm1StateWait(VirtualChannelReception* setMyVC);
  /**
   * AD Frames are always discarded.
   * If the frame number is in the lockout window, the state changes to Farm1StateLockout.
   * @param frame	The frame to handle.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return	Always returns FARM_IN_WAIT.
   */
  ReturnValue_t handleADFrame(TcTransferFrame* frame, ClcwIF* clcw);
  /**
   * These commands are handled as specified.
   * State changes to Farm1StateOpen.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return	As the frame needs no forwarding to a MAP Channel, #BC_IS_UNLOCK_COMMAND
   * 			is returned.
   */
  ReturnValue_t handleBCUnlockCommand(ClcwIF* clcw);
  /**
   * These commands are handled as specified.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @param vr The V(r) value found in the frame.
   * @return	As the frame needs no forwarding to a MAP Channel, #BC_IS_SET_VR_COMMAND
   * 			is returned.
   */
  ReturnValue_t handleBCSetVrCommand(ClcwIF* clcw, uint8_t vr);
};

#endif /* FARM1STATEWAIT_H_ */
