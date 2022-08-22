#ifndef DATALINKLAYER_H_
#define DATALINKLAYER_H_

#include <map>

#include "CCSDSReturnValuesIF.h"
#include "ClcwIF.h"
#include "TcTransferFrame.h"
#include "VirtualChannelReceptionIF.h"
#include "dllConf.h"
#include "fsfw/events/Event.h"

class VirtualChannelReception;
/**
 * A complete representation of the CCSDS Data Link Layer.
 * The operations of this layer are defined in the CCSDS TC Space Data Link Protocol
 * document. It is configured to handle a VC Demultiplexing function. All reception
 * steps are performed.
 */
class DataLinkLayer : public CCSDSReturnValuesIF {
 public:
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SYSTEM_1;
  //! [EXPORT] : [COMMENT] A RF available signal was detected. P1: raw RFA state, P2: 0
  static const Event RF_AVAILABLE = MAKE_EVENT(0, severity::INFO);
  //! [EXPORT] : [COMMENT] A previously found RF available signal was lost.
  //! P1: raw RFA state, P2: 0
  static const Event RF_LOST = MAKE_EVENT(1, severity::INFO);
  //! [EXPORT] : [COMMENT] A Bit Lock signal. Was detected. P1: raw BLO state, P2: 0
  static const Event BIT_LOCK = MAKE_EVENT(2, severity::INFO);
  //! [EXPORT] : [COMMENT] A previously found Bit Lock signal was lost. P1: raw BLO state, P2: 0
  static const Event BIT_LOCK_LOST = MAKE_EVENT(3, severity::INFO);
  //	static const Event RF_CHAIN_LOST = MAKE_EVENT(4, severity::INFO); //!< The CCSDS Board
  // detected that either bit lock or RF available or both are lost. No parameters.
  //! [EXPORT] : [COMMENT] The CCSDS Board could not interpret a TC
  static const Event FRAME_PROCESSING_FAILED = MAKE_EVENT(5, severity::LOW);
  /**
   * The Constructor sets the passed parameters and nothing else.
   * @param set_frame_buffer The buffer in which incoming frame candidates are stored.
   * @param setClcw	The CLCW class to work on when returning CLCW information.
   * @param set_start_sequence_length	Length of the Start sequence in front of every TC Transfer
   * Frame.
   * @param set_scid	The SCID to operate on.
   */
  DataLinkLayer(uint8_t* set_frame_buffer, ClcwIF* setClcw, uint8_t set_start_sequence_length,
                uint16_t set_scid);
  /**
   * Empty virtual destructor.
   */
  ~DataLinkLayer();
  /**
   * This method tries to process a frame that is placed in #frameBuffer.
   * The procedures described in the Standard are performed.
   * @param length	Length of the incoming frame candidate.
   * @return	@c returnvalue::OK on successful handling, otherwise the return codes of the higher
   * methods.
   */
  ReturnValue_t processFrame(uint16_t length);
  /**
   * Configuration method to add a new TC Virtual Channel.
   * Shall only be called during initialization. As soon as the method was called, the layer can
   * handle Frames directed to this VC.
   * @param virtualChannelId	Id of the VC. Shall be smaller than 64.
   * @param object	Reference to the object that handles the Frame.
   * @return	@c returnvalue::OK on success, @c returnvalue::FAILED otherwise.
   */
  ReturnValue_t addVirtualChannel(uint8_t virtualChannelId, VirtualChannelReceptionIF* object);
  /**
   * The initialization method calls the @c initialize routine of all virtual channels.
   * @return The return code of the first failed VC initialization or @c returnvalue::OK.
   */
  ReturnValue_t initialize();

 private:
  typedef std::map<uint8_t, VirtualChannelReceptionIF*>::iterator
      virtualChannelIterator;  //!< Typedef to simplify handling the #virtualChannels map.
  static const uint8_t FRAME_VERSION_NUMBER_DEFAULT =
      0x00;  //!< Constant for the default value of Frame Version Numbers.
  static const uint8_t FRAME_PRIMARY_HEADER_LENGTH = 5;  //!< Length of the frame's primary header.
  static const uint8_t START_SEQUENCE_PATTERN =
      0x00;                          //!< The start sequence pattern which might be with the frame.
  static const bool USE_CRC = true;  //!< A global, so called "Managed Parameter" that identifies if
                                     //!< incoming frames have CRC's or not.
  uint16_t spacecraftId;             //!< The Space Craft Identifier (SCID) configured.
  uint8_t* frameBuffer;              //!< A pointer to point to the current incoming frame.
  ClcwIF* clcw;                      //!< Pointer to store the CLCW to work on.
  uint16_t receivedDataLength;       //!< Stores the length of the currently processed frame.
  TcTransferFrame currentFrame;      //!< Stores a more convenient access to the current frame.
  uint8_t startSequenceLength;  //!< Configured length of the start sequence. Maybe this must be
                                //!< done more variable.
  std::map<uint8_t, VirtualChannelReceptionIF*>
      virtualChannels;  //!< Map of all virtual channels assigned.
  /**
   * Method that performs all possible frame validity checks (as specified).
   * @return	Various error codes or @c returnvalue::OK on success.
   */
  ReturnValue_t frameValidationCheck();
  /**
   * First method to call.
   * Removes start sequence bytes and checks if the complete frame was received.
   * SHOULDDO: Maybe handling the start sequence must be done more variable.
   * @return @c returnvalue::OK or @c TOO_SHORT.
   */
  ReturnValue_t frameDelimitingAndFillRemoval();
  /**
   * Small helper method to check the CRC of the Frame.
   * @return @c returnvalue::OK or @c CRC_FAILED.
   */
  ReturnValue_t frameCheckCRC();
  /**
   * Method that groups the reception process of all Frames.
   * Calls #frameDelimitingAndFillRemoval and #frameValidationCheck.
   * @return The return codes of the sub calls.
   */
  ReturnValue_t allFramesReception();
  /**
   * Dummy method for master channel demultiplexing.
   * As there's only one Master Channel here, the method calls #virtualChannelDemultiplexing.
   * @return	The return codes of #virtualChannelDemultiplexing.
   */
  ReturnValue_t masterChannelDemultiplexing();
  /**
   * Method to demultiplex the Frames to Virtual Channels (VC's).
   * Looks up the requested VC in #virtualChannels and forwards the Frame to its
   * #frameAcceptanceAndReportingMechanism method, if found.
   * @return The higher method codes or @c VC_NOT_FOUND.
   */
  ReturnValue_t virtualChannelDemultiplexing();
};

#endif /* DATALINKLAYER_H_ */
