#ifndef FSFW_SRC_FSFW_CFDP_TLV_FLOWLABELTLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_FLOWLABELTLV_H_

#include "Tlv.h"

class FlowLabelTlv : public cfdp::Tlv {
 public:
  FlowLabelTlv(uint8_t* value, size_t size);

 private:
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_FLOWLABELTLV_H_ */
