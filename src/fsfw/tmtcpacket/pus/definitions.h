#ifndef FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_
#define FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_

#include <cstdint>

#include "fsfw/serialize/SerializeIF.h"

namespace ecss {

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

  [[nodiscard]] size_t getLength() const {
    if (type == DataTypes::RAW) {
      return dataUnion.raw.len;
    } else if (type == DataTypes::SERIALIZABLE and dataUnion.serializable != nullptr) {
      return dataUnion.serializable->getSerializedSize();
    }
    return 0;
  }
};

}  // namespace ecss

#endif /* FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_ */
