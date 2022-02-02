#ifndef CRC_CCITT_H_
#define CRC_CCITT_H_

#include <stdint.h>

class CRC {
 public:
  static uint16_t crc16ccitt(uint8_t const input[], uint32_t length, uint16_t startingCrc = 0xffff);

 private:
  CRC();

  static const uint16_t crc16ccitt_table[256];
};

#endif /* CRC_H_ */
