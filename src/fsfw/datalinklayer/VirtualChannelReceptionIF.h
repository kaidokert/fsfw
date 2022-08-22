/**
 * @file	VirtualChannelReceptionIF.h
 * @brief	This file defines the VirtualChannelReceptionIF class.
 * @date	25.03.2013
 * @author	baetz
 */

#ifndef VIRTUALCHANNELRECEPTIONIF_H_
#define VIRTUALCHANNELRECEPTIONIF_H_

#include "ClcwIF.h"
#include "TcTransferFrame.h"
#include "dllConf.h"
#include "fsfw/returnvalues/returnvalue.h"

/**
 * This is the interface for Virtual Channel reception classes.
 * It represents a single TC Virtual Channel that operates on one IO
 */
class VirtualChannelReceptionIF {
 public:
  /**
   * Enum including all valid types of frames.
   * The type is made up by two flags, so 0b1111 is definitely illegal.
   */
  enum frameType { AD_FRAME = 0b00, BC_FRAME = 0b11, BD_FRAME = 0b10, ILLEGAL_FRAME = 0b1111 };
  /**
   * Empty virtual destructor.
   */
  virtual ~VirtualChannelReceptionIF() {}
  /**
   * This method shall accept frames and do all FARM-1 stuff.
   * Handling the Frame includes forwarding to higher-level procedures.
   * @param frame	The Tc Transfer Frame that was received and checked.
   * @param clcw	Any changes to the CLCW value are forwarded by using this parameter.
   * @return The return Value shall indicate successful processing with @c returnvalue::OK.
   */
  virtual ReturnValue_t frameAcceptanceAndReportingMechanism(TcTransferFrame* frame,
                                                             ClcwIF* clcw) = 0;
  /**
   * If any other System Objects are required for operation they shall be initialized here.
   * @return	@c returnvalue::OK for successful initialization.
   */
  virtual ReturnValue_t initialize() = 0;
  /**
   * Getter for the VCID.
   * @return	The #channelId.
   */
  virtual uint8_t getChannelId() const = 0;
};

#endif /* VIRTUALCHANNELRECEPTIONIF_H_ */
