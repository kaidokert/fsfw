#include "MessageToUserTlv.h"

MessageToUserTlv::MessageToUserTlv(uint8_t* value, size_t size)
    : Tlv(cfdp::TlvType::MSG_TO_USER, value, size) {}

MessageToUserTlv::MessageToUserTlv() : Tlv() {}

MessageToUserTlv::MessageToUserTlv(const std::vector<uint8_t>& data)
    : Tlv(cfdp::TlvType::MSG_TO_USER, data.data(), data.size()) {}
