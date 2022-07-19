#ifndef FSFW_TMTCPACKET_PUSTMIF_H
#define FSFW_TMTCPACKET_PUSTMIF_H

#include <cstddef>
#include <cstdint>

#include "fsfw/tmtcpacket/pus/PusIF.h"

class PusTmIF : public PusIF {
 public:
  ~PusTmIF() override = default;

  virtual uint8_t getScTimeRefStatus() = 0;
  virtual uint16_t getMessageTypeCounter() = 0;
  virtual uint16_t getDestId() = 0;
  virtual const uint8_t* getTimestamp(size_t& timeStampLen) = 0;
  virtual size_t getTimestampLen() = 0;
};

#endif  // FSFW_TMTCPACKET_PUSTMIF_H
