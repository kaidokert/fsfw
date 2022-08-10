#ifndef FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_

#include <vector>

#include "Tlv.h"

class MessageToUserTlv : public cfdp::Tlv {
 public:
  MessageToUserTlv();
  MessageToUserTlv(uint8_t* value, size_t size);
  explicit MessageToUserTlv(const std::vector<uint8_t>& data);

 private:
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_MESSAGETOUSERTLV_H_ */
