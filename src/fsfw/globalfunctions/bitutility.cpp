#include "fsfw/globalfunctions/bitutility.h"

void bitutil::set(uint8_t *byte, uint8_t position) {
  if (position > 7) {
    return;
  }
  uint8_t shiftNumber = position + (7 - 2 * position);
  *byte |= 1 << shiftNumber;
}

void bitutil::toggle(uint8_t *byte, uint8_t position) {
  if (position > 7) {
    return;
  }
  uint8_t shiftNumber = position + (7 - 2 * position);
  *byte ^= 1 << shiftNumber;
}

void bitutil::clear(uint8_t *byte, uint8_t position) {
  if (position > 7) {
    return;
  }
  uint8_t shiftNumber = position + (7 - 2 * position);
  *byte &= ~(1 << shiftNumber);
}

bool bitutil::get(const uint8_t *byte, uint8_t position, bool &bit) {
  if (position > 7) {
    return false;
  }
  uint8_t shiftNumber = position + (7 - 2 * position);
  bit = *byte & (1 << shiftNumber);
  return true;
}
