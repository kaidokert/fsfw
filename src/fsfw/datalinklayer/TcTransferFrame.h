#ifndef TCTRANSFERFRAME_H_
#define TCTRANSFERFRAME_H_

#include <cstddef>
#include <cstdint>

#include "dllConf.h"

/**
 * The TcTransferFrame class simplifies handling of such Frames.
 * It operates on any buffer passed on construction. The data length
 * is determined by the length field in the frame itself.
 * It has a lot of getters for convenient access to the content.
 * @ingroup ccsds_handling
 */
class TcTransferFrame {
 protected:
  /**
   * The struct that defines the Frame's Primary Header.
   */
  struct TcTransferFramePrimaryHeader {
    uint8_t flagsAndScid;     //!< Highest byte with Flags and part of SCID.
    uint8_t spacecraftId_l;   //!< Byte with rest of SCID
    uint8_t vcidAndLength_h;  //!< Byte with VCID and part of length.
    uint8_t length_l;         //!< Byte with rest of length.
    uint8_t sequenceNumber;   //!< Lowest byte with Frame Sequence Number N(S).
  };
  /**
   * The struct defining the whole Transfer Frame.
   */
  struct tc_transfer_frame {
    TcTransferFramePrimaryHeader header;  //!< The header struct.
    uint8_t dataField;                    //!< The data field of the Transfer Frame.
  };
  tc_transfer_frame* frame;  //!< Pointer to a buffer where a Frame is placed.
 public:
  static const uint8_t FRAME_CRC_SIZE = 2;  //!< Constant for the CRC size.
  /**
   * Empty Constructor that sets the data pointer to NULL.
   */
  TcTransferFrame();
  /**
   * The data pointer passed in this Constructor is casted to the #tc_transfer_frame struct.
   * @param setData The data on which the class shall operate.
   */
  TcTransferFrame(uint8_t* setData);
  /**
   * Getter.
   * @return The Version number.
   */
  uint8_t getVersionNumber();
  /**
   * Getter.
   * @return	If the bypass flag is set or not.
   */
  bool bypassFlagSet();
  /**
   * Getter.
   * @return If the control command flag is set or not.
   */
  bool controlCommandFlagSet();
  /**
   * Getter.
   * @return If the spare bits in the Header are zero or not.
   */
  bool spareIsZero();
  /**
   * Getter.
   * @return The Spacecraft Identifier.
   */
  uint16_t getSpacecraftId();
  /**
   * Getter.
   * @return The Virtual Channel Identifier.
   */
  uint8_t getVirtualChannelId();
  /**
   * Getter.
   * @return The Frame length as stored in the Header.
   */
  uint16_t getFrameLength();
  /**
   * Getter.
   * @return The length of pure data (without CRC), assuming that a Segment Header is present.
   */
  uint16_t getDataLength();
  /**
   * Getter.
   * @return The length of pure data (without CRC), assuming that no Segment Header is present (for
   * BC Frames).
   */
  uint16_t getFullDataLength();
  /**
   * Getter.
   * @return The sequence number from the header.
   */
  uint8_t getSequenceNumber();
  /**
   * Getter.
   * @return The Sequence Flags in the Segment Header byte (right aligned).
   */
  uint8_t getSequenceFlags();
  /**
   * Getter.
   * @return The Multiplexer Access Point Identifier from the Segment Header byte.
   */
  uint8_t getMAPId();
  /**
   * Getter.
   * @return A pointer to the date field AFTER a Segment Header.
   */
  uint8_t* getDataField();
  /**
   * Getter.
   * @return A pointer to the first byte in the Data Field (ignoring potential Segment Headers, for
   * BC Frames).
   */
  uint8_t* getFullDataField();
  /**
   * Getter.
   * @return A pointer to the beginning of the Frame.
   */
  uint8_t* getFullFrame();
  /**
   * Getter
   * @return The total size of the Frame, which is the size stated in the Header + 1.
   */
  uint16_t getFullSize();
  /**
   * Getter.
   * @return Size of the #TcTransferFramePrimaryHeader.
   */
  uint16_t getHeaderSize();
  /**
   * Debug method to print the whole Frame to screen.
   */
  void print();
};

#endif /* TCTRANSFERFRAME_H_ */
