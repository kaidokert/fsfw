#ifndef FSFW_GLOBALFUNCTIONS_BITUTIL_H_
#define FSFW_GLOBALFUNCTIONS_BITUTIL_H_

#include <cstdint>

namespace bitutil {

/* Helper functions for manipulating the individual bits of a byte.
Position refers to n-th bit of a byte, going from 0 (most significant bit) to
7 (least significant bit) */
void bitSet(uint8_t* byte, uint8_t position);
void bitToggle(uint8_t* byte, uint8_t position);
void bitClear(uint8_t* byte, uint8_t position);
bool bitGet(const uint8_t* byte, uint8_t position);

}

#endif /* FSFW_GLOBALFUNCTIONS_BITUTIL_H_ */
