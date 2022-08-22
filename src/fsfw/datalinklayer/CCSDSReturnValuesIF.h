/**
 * @file	CCSDSReturnValuesIF.h
 * @brief	This file defines the CCSDSReturnValuesIF class.
 * @date	24.04.2013
 * @author	baetz
 */

#ifndef CCSDSRETURNVALUESIF_H_
#define CCSDSRETURNVALUESIF_H_

#include "dllConf.h"
#include "fsfw/returnvalues/returnvalue.h"
/**
 * This is a helper class to collect special return values that come up during CCSDS Handling.
 * @ingroup ccsds_handling
 */
class CCSDSReturnValuesIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::CCSDS_HANDLER_IF;  //!< Basic ID of the interface.

  static const ReturnValue_t BC_IS_SET_VR_COMMAND =
      MAKE_RETURN_CODE(0x01);  //!< A value to describe a BC frame.
  static const ReturnValue_t BC_IS_UNLOCK_COMMAND =
      MAKE_RETURN_CODE(0x02);  //!< A value to describe a BC frame.
  static const ReturnValue_t BC_ILLEGAL_COMMAND =
      MAKE_RETURN_CODE(0xB0);  //!< A value to describe an illegal BC frame.
  static const ReturnValue_t BOARD_READING_NOT_FINISHED = MAKE_RETURN_CODE(
      0xB1);  //! The CCSDS Board is not yet finished reading, it requires another cycle.

  static const ReturnValue_t NS_POSITIVE_W =
      MAKE_RETURN_CODE(0xF0);  //!< NS is in the positive window
  static const ReturnValue_t NS_NEGATIVE_W =
      MAKE_RETURN_CODE(0xF1);                                      //!< NS is in the negative window
  static const ReturnValue_t NS_LOCKOUT = MAKE_RETURN_CODE(0xF2);  //!< NS is in lockout state
  static const ReturnValue_t FARM_IN_LOCKOUT =
      MAKE_RETURN_CODE(0xF3);  //!< FARM-1 is currently in lockout state
  static const ReturnValue_t FARM_IN_WAIT =
      MAKE_RETURN_CODE(0xF4);  //!< FARM-1 is currently in wait state

  static const ReturnValue_t WRONG_SYMBOL =
      MAKE_RETURN_CODE(0xE0);  //!< An error code in the FrameFinder.
  static const ReturnValue_t DOUBLE_START =
      MAKE_RETURN_CODE(0xE1);  //!< An error code in the FrameFinder.
  static const ReturnValue_t START_SYMBOL_MISSED =
      MAKE_RETURN_CODE(0xE2);  //!< An error code in the FrameFinder.
  static const ReturnValue_t END_WITHOUT_START =
      MAKE_RETURN_CODE(0xE3);  //!< An error code in the FrameFinder.
  static const ReturnValue_t TOO_LARGE = MAKE_RETURN_CODE(0xE4);  //!< An error code for a frame.
  static const ReturnValue_t TOO_SHORT = MAKE_RETURN_CODE(0xE5);  //!< An error code for a frame.
  static const ReturnValue_t WRONG_TF_VERSION =
      MAKE_RETURN_CODE(0xE6);  //!< An error code for a frame.
  static const ReturnValue_t WRONG_SPACECRAFT_ID =
      MAKE_RETURN_CODE(0xE7);  //!< An error code for a frame.
  static const ReturnValue_t NO_VALID_FRAME_TYPE =
      MAKE_RETURN_CODE(0xE8);                                        //!< An error code for a frame.
  static const ReturnValue_t CRC_FAILED = MAKE_RETURN_CODE(0xE9);    //!< An error code for a frame.
  static const ReturnValue_t VC_NOT_FOUND = MAKE_RETURN_CODE(0xEA);  //!< An error code for a frame.
  static const ReturnValue_t FORWARDING_FAILED =
      MAKE_RETURN_CODE(0xEB);  //!< An error code for a frame.
  static const ReturnValue_t CONTENT_TOO_LARGE =
      MAKE_RETURN_CODE(0xEC);  //!< An error code for a frame.
  static const ReturnValue_t RESIDUAL_DATA =
      MAKE_RETURN_CODE(0xED);  //!< An error code for a frame.
  static const ReturnValue_t DATA_CORRUPTED =
      MAKE_RETURN_CODE(0xEE);  //!< An error code for a frame.
  static const ReturnValue_t ILLEGAL_SEGMENTATION_FLAG =
      MAKE_RETURN_CODE(0xEF);  //!< An error code for a frame.
  static const ReturnValue_t ILLEGAL_FLAG_COMBINATION =
      MAKE_RETURN_CODE(0xD0);  //!< An error code for a frame.
  static const ReturnValue_t SHORTER_THAN_HEADER =
      MAKE_RETURN_CODE(0xD1);  //!< An error code for a frame.
  static const ReturnValue_t TOO_SHORT_BLOCKED_PACKET =
      MAKE_RETURN_CODE(0xD2);  //!< An error code for a frame.
  static const ReturnValue_t TOO_SHORT_MAP_EXTRACTION =
      MAKE_RETURN_CODE(0xD3);  //!< An error code for a frame.

  virtual ~CCSDSReturnValuesIF() {}  //!< Empty virtual destructor
};

#endif /* CCSDSRETURNVALUESIF_H_ */
