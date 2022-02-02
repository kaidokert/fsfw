/**
 * @file	BCFrame.h
 * @brief	This file defines the BCFrame class.
 * @date	24.04.2013
 * @author	baetz
 */

#ifndef BCFRAME_H_
#define BCFRAME_H_

#include "CCSDSReturnValuesIF.h"
#include "dllConf.h"

/**
 * Small helper class to identify a BcFrame.
 * @ingroup ccsds_handling
 */
class BcFrame : public CCSDSReturnValuesIF {
 private:
  static const uint8_t UNLOCK_COMMAND = 0b00000000;  //! Identifier for a certain BC Command.
  static const uint8_t SET_V_R_1 = 0b10000010;       //! Identifier for a certain BC Command.
  static const uint8_t SET_V_R_2 = 0b00000000;       //! Identifier for a certain BC Command.

 public:
  uint8_t byte1;  //!< First content byte
  uint8_t byte2;  //!< Second content byte
  uint8_t vR;     //!< vR byte
  /**
   * Simple default constructor.
   */
  BcFrame() : byte1(0), byte2(0), vR(0) {}
  /**
   * Main and only useful method of the class.
   * With the buffer and size information passed, the class passes the content
   * and checks if it is one of the two valid BC Command Frames.
   * @param inBuffer	Content of the frame to check,
   * @param inSize	Size of the data to check.
   * @return 	- #BC_ILLEGAL_COMMAND if it is no command.
   * 			- #BC_IS_UNLOCK_COMMAND if it is an unlock command.
   * 			- #BC_IS_SET_VR_COMMAND if it is such.
   */
  ReturnValue_t initialize(const uint8_t* inBuffer, uint16_t inSize) {
    ReturnValue_t returnValue = BC_ILLEGAL_COMMAND;
    if (inSize == 1) {
      byte1 = inBuffer[0];
      if (byte1 == UNLOCK_COMMAND) {
        returnValue = BC_IS_UNLOCK_COMMAND;
      }
    } else if (inSize == 3) {
      byte1 = inBuffer[0];
      byte2 = inBuffer[1];
      vR = inBuffer[2];
      if (byte1 == SET_V_R_1 && byte2 == SET_V_R_2) {
        returnValue = BC_IS_SET_VR_COMMAND;
      }
    }
    return returnValue;
  }
};

#endif /* BCFRAME_H_ */
