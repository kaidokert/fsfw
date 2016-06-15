/*
 * crc.h
 *
 *  Created on: 03.04.2012
 *      Author: bucher
 */

#ifndef CRC_CCITT_H_
#define CRC_CCITT_H_

#include <stdint.h>

uint16_t Calculate_CRC(uint8_t const input[], uint32_t length);


#endif /* CRC_H_ */
