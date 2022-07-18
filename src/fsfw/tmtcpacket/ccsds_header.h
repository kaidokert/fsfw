#ifndef CCSDS_HEADER_H_
#define CCSDS_HEADER_H_

#include <cstdint>

struct CCSDSPrimaryHeader {
  uint8_t packetIdHAndVersion;
  uint8_t packetIdL;
  uint8_t packetSeqCtrlH;
  uint8_t packetSeqCtrlL;
  uint8_t packetLenH;
  uint8_t packetLenL;
};

#endif /* CCSDS_HEADER_H_ */
