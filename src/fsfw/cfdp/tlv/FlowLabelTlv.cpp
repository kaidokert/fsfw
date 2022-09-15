#include "FlowLabelTlv.h"

FlowLabelTlv::FlowLabelTlv(uint8_t* value, size_t size)
    : Tlv(cfdp::TlvType::FLOW_LABEL, value, size) {}
