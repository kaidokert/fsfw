#ifndef FSFW_UTIL_DATAWRAPPER_H
#define FSFW_UTIL_DATAWRAPPER_H

#include <cstddef>
#include <cstdint>
#include <utility>

#include "fsfw/serialize.h"

namespace util {

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

}  // namespace util

#endif  // FSFW_UTIL_DATAWRAPPER_H
