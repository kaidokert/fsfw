/**
 * @file	Farm1StateIF.h
 * @brief	This file defines the Farm1StateIF class.
 * @date	24.04.2013
 * @author	baetz
 */

#ifndef FARM1STATEIF_H_
#define FARM1STATEIF_H_

#include "CCSDSReturnValuesIF.h"
#include "dllConf.h"

class VirtualChannelReception;
class TcTransferFrame;
class ClcwIF;

/**
 * This is the interface for states of the FARM-1 state machine.
 * Classes implementing this interface can be used as FARM-1 states. This is a simple implementation
 * of the state pattern.
 */
class Farm1StateIF : public CCSDSReturnValuesIF {
 public:
  /**
   * A method that shall handle an incoming frame as AD Frame.
   * @param frame	The frame to handle.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return	If forwarding to a MAP Channel is required, the return value shall be #FRAME_OK.
   * 			Otherwise, an appropriate return value or error code shall be generated.
   */
  virtual ReturnValue_t handleADFrame(TcTransferFrame* frame, ClcwIF* clcw) = 0;
  /**
   * This method shall handle frames that have been successfully identified as BC Unlock frames.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @return	If forwarding to a MAP Channel is required, the return value shall be #FRAME_OK.
   * 			Otherwise, an appropriate return value or error code shall be generated.
   */
  virtual ReturnValue_t handleBCUnlockCommand(ClcwIF* clcw) = 0;
  /**
   * This method shall handle frames that have been successfully identified as BC Set VR frames.
   * @param clcw	Any changes to the CLCW shall be done with the help of this interface.
   * @param vr The V(r) value found in the frame.
   * @return	If forwarding to a MAP Channel is required, the return value shall be #FRAME_OK.
   * 			Otherwise, an appropriate return value or error code shall be generated.
   */
  virtual ReturnValue_t handleBCSetVrCommand(ClcwIF* clcw, uint8_t vr) = 0;
  /**
   * Empty virtual destructor.
   */
  virtual ~Farm1StateIF() {}
};

#endif /* FARM1STATEIF_H_ */
