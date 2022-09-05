#ifndef FSFW_TMTCPACKET_CREATORDATAIF_H
#define FSFW_TMTCPACKET_CREATORDATAIF_H

#include "defs.h"

class CustomUserDataIF {
 public:
  virtual ~CustomUserDataIF() = default;
  virtual ReturnValue_t setRawUserData(const uint8_t* data, size_t len) = 0;
  virtual ReturnValue_t setSerializableUserData(const SerializeIF& serializable) = 0;
};
#endif  // FSFW_TMTCPACKET_CREATORDATAIF_H
