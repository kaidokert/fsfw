#ifndef FSFW_TMTCPACKET_RAWDATAIF_H
#define FSFW_TMTCPACKET_RAWDATAIF_H

#include <cstddef>
#include <cstdint>

class RawUserDataReaderIF {
 public:
  ~RawUserDataReaderIF() = default;
  [[nodiscard]] virtual const uint8_t* getUserData() const = 0;
  [[nodiscard]] virtual size_t getUserDataLen() const = 0;
};

#endif  // FSFW_TMTCPACKET_RAWDATAIF_H
