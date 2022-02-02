#ifndef FSFW_GLOBALFUNCTIONS_BITUTIL_H_
#define FSFW_GLOBALFUNCTIONS_BITUTIL_H_

#include <cstdint>

namespace bitutil {

// Helper functions for manipulating the individual bits of a byte.
// Position refers to n-th bit of a byte, going from 0 (most significant bit) to
// 7 (least significant bit)

/**
 * @brief   Set the bit in a given byte
 * @param byte
 * @param position
 */
void set(uint8_t* byte, uint8_t position);
/**
 * @brief   Toggle the bit in a given byte
 * @param byte
 * @param position
 */
void toggle(uint8_t* byte, uint8_t position);
/**
 * @brief   Clear the bit in a given byte
 * @param byte
 * @param position
 */
void clear(uint8_t* byte, uint8_t position);
/**
 * @brief   Get the bit in a given byte
 * @param byte
 * @param position
 * @param If the input is valid, this will be set to true if the bit is set and false otherwise.
 * @return False if position is invalid, True otherwise
 */
bool get(const uint8_t* byte, uint8_t position, bool& bit);

}  // namespace bitutil

#endif /* FSFW_GLOBALFUNCTIONS_BITUTIL_H_ */
