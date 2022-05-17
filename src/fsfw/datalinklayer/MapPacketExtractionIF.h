/**
 * @file	MapPacketExtractionIF.h
 * @brief	This file defines the MapPacketExtractionIF class.
 * @date	25.03.2013
 * @author	baetz
 */

#ifndef MAPPACKETEXTRACTIONIF_H_
#define MAPPACKETEXTRACTIONIF_H_

#include "CCSDSReturnValuesIF.h"
#include "TcTransferFrame.h"
#include "dllConf.h"

/**
 * This is the interface for MAP Packet Extraction classes.
 * All classes implementing this interface shall be able to extract blocked or segmented Space
 * Packets on a certain MAP channel. This is done in accordance with the CCSDS TC Space Data Link
 * Protocol.
 */
class MapPacketExtractionIF : public CCSDSReturnValuesIF {
 protected:
  static const uint8_t FIRST_PORTION =
      0b01;  //!< Identification of the first part of a segmented Packet.
  static const uint8_t CONTINUING_PORTION =
      0b00;  //!< Identification of a continuing part of segmented Packets.
  static const uint8_t LAST_PORTION = 0b10;  //!< The last portion of a segmented Packet.
  static const uint8_t NO_SEGMENTATION =
      0b11;  //!< A Frame without segmentation but maybe with blocking.
 public:
  /**
   * Empty virtual destructor.
   */
  virtual ~MapPacketExtractionIF() {}
  /**
   * Method to call to handle a single Transfer Frame.
   * The method tries to extract Packets from the frame as stated in the Standard.
   * @param frame
   * @return
   */
  virtual ReturnValue_t extractPackets(TcTransferFrame* frame) = 0;
  /**
   * Any post-instantiation initialization shall be done in this method.
   * @return
   */
  virtual ReturnValue_t initialize() = 0;
};

#endif /* MAPPACKETEXTRACTIONIF_H_ */
