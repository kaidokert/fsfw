#ifndef FSFW_SRC_FSFW_CFDP_TLVIF_H_
#define FSFW_SRC_FSFW_CFDP_TLVIF_H_

#include "../definitions.h"

class TlvIF : public SerializeIF {
 public:
  virtual ~TlvIF(){};

  virtual uint8_t getLengthField() const = 0;
  virtual cfdp::TlvType getType() const = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_TLVIF_H_ */
