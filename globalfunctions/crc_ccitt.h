#ifndef CRC_CCITT_H_
#define CRC_CCITT_H_

#include <stdint.h>

uint16_t Calculate_CRC(uint8_t const input[], uint32_t length, uint16_t startingCrc = 0xffff);


#endif /* CRC_H_ */
