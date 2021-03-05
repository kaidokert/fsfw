#include "bitutility.h"

void bitutil::bitSet(uint8_t *byte, uint8_t position) {
    if(position > 7) {
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte |= 1 << shiftNumber;
}

void bitutil::bitToggle(uint8_t *byte, uint8_t position) {
    if(position > 7) {
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte ^= 1 << shiftNumber;
}

void bitutil::bitClear(uint8_t *byte, uint8_t position) {
    if(position > 7) {
        return;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    *byte &= ~(1 << shiftNumber);
}

bool bitutil::bitGet(const uint8_t *byte, uint8_t position) {
    if(position > 7) {
        return false;
    }
    uint8_t shiftNumber = position + (7 - 2 * position);
    return *byte & (1 << shiftNumber);
}
