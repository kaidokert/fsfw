/**
 * @file	Farm1StateOpen.h
 * @brief	This file defines the Farm1StateOpen class.
 * @date	24.04.2013
 * @author	baetz
 */

#ifndef FARM1STATEOPEN_H_
#define FARM1STATEOPEN_H_

#include "Farm1StateIF.h"
#include "dllConf.h"

/**
 * This class represents the FARM-1 "Open" State.
 * The Open state is the state of normal operation. It handles all types of frames,
 * including AD Frames. If a wrong Frame Sequence Number is detected in an AD Frame, the
 * State reacts as specified.
 */
class Farm1StateOpen : public Farm1StateIF {
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
  Farm1StateOpen(VirtualChannelReception* setMyVC);
  /**
   * Method to check the validity of AD Frames.
   * It checks the Frame Sequence Number and reacts as specified in the standard. The state may
   * change to Farm1StateLockout.
   * @param frame	The frame to handle.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return If the Sequence Number is ok, it returns returnvalue::OK. Otherwise either
   * #NS_POSITIVE_W, #NS_NEGATIVE_W or NS_LOCKOUT is returned.
   */
  ReturnValue_t handleADFrame(TcTransferFrame* frame, ClcwIF* clcw);
  /**
   * These commands are handled as specified.
   * State does not change.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return	As the frame needs no forwarding to a MAP Channel, #BC_IS_UNLOCK_COMMAND
   * 			is returned.
   */
  ReturnValue_t handleBCUnlockCommand(ClcwIF* clcw);
  /**
   * These commands are handled as specified.
   * State does not change.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @param vr The V(r) value found in the frame.
   * @return	As the frame needs no forwarding to a MAP Channel, #BC_IS_SET_VR_COMMAND
   * 			is returned.
   */
  ReturnValue_t handleBCSetVrCommand(ClcwIF* clcw, uint8_t vr);
};

#endif /* FARM1STATEOPEN_H_ */
