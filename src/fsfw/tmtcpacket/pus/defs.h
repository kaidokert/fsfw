#ifndef FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_
#define FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_

#include <cstdint>
#include <utility>

#include "fsfw/serialize/SerializeIF.h"

namespace ecss {

using PusChecksumT = uint16_t;

//! Version numbers according to ECSS-E-ST-70-41C p.439
enum PusVersion : uint8_t { PUS_A = 1, PUS_C = 2 };

struct RawData {
  const uint8_t* data;
  size_t len;
};

enum DataTypes { RAW, SERIALIZABLE };

union DataUnion {
  RawData raw;
  SerializeIF* serializable;
};

struct DataWrapper {
  DataTypes type;
  DataUnion dataUnion;
  using BufPairT = std::pair<const uint8_t*, size_t>;

  [[nodiscard]] size_t getLength() const {
    if (type == DataTypes::RAW) {
      return dataUnion.raw.len;
    } else if (type == DataTypes::SERIALIZABLE and dataUnion.serializable != nullptr) {
      return dataUnion.serializable->getSerializedSize();
    }
    return 0;
  }

  void setRawData(BufPairT bufPair) {
    type = DataTypes::RAW;
    dataUnion.raw.data = bufPair.first;
    dataUnion.raw.len = bufPair.second;
  }

  void setSerializable(SerializeIF& serializable) {
    type = DataTypes::SERIALIZABLE;
    dataUnion.serializable = &serializable;
  }
};

/**
 * This struct defines the data structure of a Space Packet when accessed
 * via a pointer.
 * @ingroup tmtcpackets
 */
struct PusPointers {
  const uint8_t* spHeaderStart;
  const uint8_t* secHeaderStart;
  const uint8_t* userDataStart;
  const uint8_t* crcStart;
};

}  // namespace ecss

#endif /* FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_ */
