/**
 * @file	VirtualChannelReception.h
 * @brief	This file defines the VirtualChannelReception class.
 * @date	25.03.2013
 * @author	baetz
 */

#ifndef VIRTUALCHANNELRECEPTION_H_
#define VIRTUALCHANNELRECEPTION_H_

#include <map>

#include "CCSDSReturnValuesIF.h"
#include "Clcw.h"
#include "Farm1StateIF.h"
#include "Farm1StateLockout.h"
#include "Farm1StateOpen.h"
#include "Farm1StateWait.h"
#include "MapPacketExtractionIF.h"
#include "VirtualChannelReceptionIF.h"
#include "dllConf.h"
/**
 * Implementation of a TC Virtual Channel.
 * This is a full implementation of a virtual channel as specified in the CCSDS TC Space Data Link
 * Protocol. It is designed to operate within an instance of the @c DataLinkLayer class.
 * Features:
 *  - any (6bit) Virtual Channel ID is assignable.
 *  - Supports an arbitrary number of MAP Channels (with a map).
 *  - Has a complete FARM-1 Machine built-in.
 *
 * The FARM-1 state machine uses the State Pattern.
 */
class VirtualChannelReception : public VirtualChannelReceptionIF, public CCSDSReturnValuesIF {
  friend class Farm1StateOpen;
  friend class Farm1StateWait;
  friend class Farm1StateLockout;

 private:
  uint8_t channelId;           //!< Stores the VCID that was assigned on construction.
  uint8_t slidingWindowWidth;  //!< A constant to set the FARM-1 sliding window width.
  uint8_t positiveWindow;      //!< The positive window for the FARM-1 machine.
  uint8_t negativeWindow;      //!< The negative window for the FARM-1 machine.
 protected:
  Farm1StateIF* currentState;  //!< The current state. To change, one of the other states must be
                               //!< assigned to this pointer.
  Farm1StateOpen openState;    //!< Instance of the FARM-1 State "Open".
  Farm1StateWait waitState;    //!< Instance of the FARM-1 State "Wait".
  Farm1StateLockout lockoutState;  //!< Instance of the FARM-1 State "Lockout".
  Clcw internalClcw;  //!< A CLCW class to internally set the values before writing them back to the
                      //!< TTC System.
  uint8_t vR;  //!< The Receiver Frame Sequence Number V(R) as it shall be maintained for every
               //!< Virtual Channel.
  uint8_t
      farmBCounter;  //!< The FARM-B COunter as it shall be maintained for every Virtual Channel.
  typedef std::map<uint8_t, MapPacketExtractionIF*>::iterator
      mapChannelIterator;  //!< Typedef to simplify handling of the mapChannels map.
  std::map<uint8_t, MapPacketExtractionIF*>
      mapChannels;  //!< A map that maintains all map Channels. Channels must be configured on
                    //!< initialization. MAy be omitted in a simplified version.
  /**
   * This method handles demultiplexing to different map channels.
   * It parses the entries of #mapChannels and forwards the Frame to a found MAP Channel.
   * @param frame	The frame to forward.
   * @return	#VC_NOT_FOUND or the return value of the map channel extraction.
   */
  ReturnValue_t mapDemultiplexing(TcTransferFrame* frame);
  /**
   * A sub-method that actually does the FARM-1 handling for different Frame types.
   * @param frame	The Tc Transfer Frame to handle.
   * @param clcw	Any changes on the CLCW shall be done with this method.
   * @return	The return code of higher methods or @c ILLEGAL_FLAG_COMBINATION.
   */
  ReturnValue_t doFARM(TcTransferFrame* frame, ClcwIF* clcw);
  /**
   * Handles incoming BD Frames.
   * Handling these Frames is independent of the State, so no subcall to #currentState is
   * required.
   * @param frame	The Tc Transfer Frame to handle.
   * @param clcw	Any changes on the CLCW shall be done with this method.
   * @return Always returns @c returnvalue::OK.
   */
  ReturnValue_t handleBDFrame(TcTransferFrame* frame, ClcwIF* clcw);
  /**
   * Handles incoming BC Frames.
   * The type of the BC Frame is detected and checked first, then methods of #currentState are
   * called.
   * @param frame	The Tc Transfer Frame to handle.
   * @param clcw	Any changes on the CLCW shall be done with this method.
   * @return The failure code of BC Frame interpretation or the return code of higher methods.
   */
  ReturnValue_t handleBCFrame(TcTransferFrame* frame, ClcwIF* clcw);

 public:
  /**
   * Default constructor.
   * Only sets the channelId of the channel. Setting the Sliding Window width is possible as well.
   * @param setChannelId	Virtual Channel Identifier (VCID) of the channel.
   */
  VirtualChannelReception(uint8_t setChannelId, uint8_t setSlidingWindowWidth);
  ReturnValue_t frameAcceptanceAndReportingMechanism(TcTransferFrame* frame, ClcwIF* clcw);
  /**
   * Helper method to simplify adding a mapChannel during construction.
   * @param mapId	The mapId of the object to add.
   * @param object	Pointer to the MapPacketExtraction object itself.
   * @return	@c returnvalue::OK if the channel was successfully inserted, @c returnvalue::FAILED
   * otherwise.
   */
  ReturnValue_t addMapChannel(uint8_t mapId, MapPacketExtractionIF* object);
  /**
   * The initialization routine checks the set #slidingWindowWidth and initializes all MAP
   * channels.
   * @return	@c returnvalue::OK on successful initialization, @c returnvalue::FAILED otherwise.
   */
  ReturnValue_t initialize();
  /**
   * Getter for the VCID.
   * @return	The #channelId.
   */
  uint8_t getChannelId() const;
  /**
   * Small method to set the state to Farm1StateWait.
   */
  void setToWaitState();
};

#endif /* VIRTUALCHANNELRECEPTION_H_ */
