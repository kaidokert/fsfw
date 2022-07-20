#ifndef FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKET_H_
#define FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKET_H_

#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

class CfdpReader : public SpacePacketReader {
 public:
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed and also
   * forwards the data pointer to the parent SpacePacketBase class.
   * @param setData	The position where the packet data lies.
   */
  explicit CfdpReader(const uint8_t* setData, size_t maxSize);
  /**
   * This is the empty default destructor.
   */
  ~CfdpReader() override;

  /**
   * This is a debugging helper method that prints the whole packet content
   * to the screen.
   */
  void print();
};

#endif /* FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKET_H_ */
