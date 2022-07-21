#ifndef FSFW_TMTCPACKET_RAWDATAIF_H
#define FSFW_TMTCPACKET_RAWDATAIF_H

#include <cstddef>
#include <cstdint>

class RawUserDataReaderIF {
 public:
  virtual const uint8_t* getUserData() const = 0;
  virtual size_t getUserDataLen() const = 0;
};

#endif  // FSFW_TMTCPACKET_RAWDATAIF_H
