#ifndef FSFW_UTIL_DATAWRAPPER_H
#define FSFW_UTIL_DATAWRAPPER_H

#include <cstddef>
#include <cstdint>
#include <utility>

#include "fsfw/serialize.h"

namespace util {

using BufPair = std::pair<const uint8_t*, size_t>;

struct RawData {
  RawData() = default;
  const uint8_t* data = nullptr;
  size_t len = 0;
};

enum DataTypes { NONE, RAW, SERIALIZABLE };

union DataUnion {
  RawData raw{};
  SerializeIF* serializable;
};

struct DataWrapper {


  DataWrapper() = default;

  DataWrapper(const uint8_t* data, size_t size): type(DataTypes::RAW) {
    setRawData({data, size});
  }

  explicit DataWrapper(BufPair raw): type(DataTypes::RAW) {
    setRawData(raw);
  }

  explicit DataWrapper(SerializeIF& serializable): type(DataTypes::SERIALIZABLE) {
    setSerializable(serializable);
  }

  DataTypes type = DataTypes::NONE;
  DataUnion dataUnion;

  [[nodiscard]] size_t getLength() const {
    if (type == DataTypes::RAW) {
      return dataUnion.raw.len;
    } else if (type == DataTypes::SERIALIZABLE and dataUnion.serializable != nullptr) {
      return dataUnion.serializable->getSerializedSize();
    }
    return 0;
  }

  [[nodiscard]] bool isNull() const {
    if ((type == DataTypes::NONE) or (type == DataTypes::RAW and dataUnion.raw.data == nullptr) or
        (type == DataTypes::SERIALIZABLE and dataUnion.serializable == nullptr)) {
      return true;
    }
    return false;
  }
  void setRawData(BufPair bufPair) {
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
