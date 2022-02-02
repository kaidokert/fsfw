#include "MessageToUserTlv.h"

MessageToUserTlv::MessageToUserTlv(uint8_t *value, size_t size):
        Tlv(cfdp::TlvTypes::MSG_TO_USER, value, size) {
}

MessageToUserTlv::MessageToUserTlv(): Tlv() {
}
