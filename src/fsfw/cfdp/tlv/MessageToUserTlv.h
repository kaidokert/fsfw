#ifndef FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_

#include "Tlv.h"

class MessageToUserTlv : public cfdp::Tlv {
 public:
  MessageToUserTlv();
  MessageToUserTlv(uint8_t* value, size_t size);

 private:
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_ */
