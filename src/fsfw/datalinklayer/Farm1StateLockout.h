#ifndef FARM1STATELOCKOUT_H_
#define FARM1STATELOCKOUT_H_

#include "Farm1StateIF.h"
#include "dllConf.h"

/**
 * This class represents the FARM-1 "Lockout" State.
 * The Lockout state is reached if the received Transfer Frame Sequence Number is completely wrong
 * (i.e. within the Lockout Window). No AD Frames are forwarded. To leave the State, a BC Unlock
 * command is required.
 */
class Farm1StateLockout : public Farm1StateIF {
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
  Farm1StateLockout(VirtualChannelReception* setMyVC);
  /**
   * All AD Frames are rejected with FARM_IN_LOCKOUT
   * @param frame	The frame to handle.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return FARM_IN_LOCKOUT
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
   * The V(r) value is not set in Lockout State, even though the Command itself is accepted.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @param vr The V(r) value found in the frame.
   * @return	As the frame needs no forwarding to a MAP Channel, #BC_IS_SET_VR_COMMAND
   * 			is returned.
   */
  ReturnValue_t handleBCSetVrCommand(ClcwIF* clcw, uint8_t vr);
};

#endif /* FARM1STATELOCKOUT_H_ */
