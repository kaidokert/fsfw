/**
 * @file	TcTransferFrameLocal.h
 * @brief	This file defines the TcTransferFrameLocal class.
 * @date	27.04.2013
 * @author	baetz
 */

#ifndef TCTRANSFERFRAMELOCAL_H_
#define TCTRANSFERFRAMELOCAL_H_

#include "TcTransferFrame.h"
#include "dllConf.h"

/**
 * This is a helper class to locally create TC Transfer Frames.
 * This is mainly required for testing purposes and therefore not very sophisticated.
 * @ingroup ccsds_handling
 */
class TcTransferFrameLocal : public TcTransferFrame {
 private:
  /**
   * A stuct to locally store the complete data.
   */
  struct frameData {
    TcTransferFramePrimaryHeader header;  //!< The primary header.
    uint8_t data[1019];                   //!< The data field.
  };

 public:
  frameData localData;  //!< The local data in the Frame.
  /**
   * The default Constructor.
   * All parameters in the Header are passed.
   * If a BC Frame is detected no segment header is created.
   * Otherwise (AD and BD), the Segment Header is set.
   * @param bypass			The bypass flag.
   * @param controlCommand	The Control Command flag.
   * @param scid				The SCID.
   * @param vcId				The VCID.
   * @param sequenceNumber	The Frame Sequence Number N(s)
   * @param setSegmentHeader	A value for the Segment Header.
   * @param data				Data to put into the Frame Data Field.
   * @param dataSize			Size of the Data.
   * @param forceCrc			if != 0, the value is used as CRC.
   */
  TcTransferFrameLocal(bool bypass, bool controlCommand, uint16_t scid, uint8_t vcId,
                       uint8_t sequenceNumber, uint8_t setSegmentHeader = 0xC0,
                       uint8_t* data = NULL, uint16_t dataSize = 0, uint16_t forceCrc = 0);
};

#endif /* TCTRANSFERFRAMELOCAL_H_ */
