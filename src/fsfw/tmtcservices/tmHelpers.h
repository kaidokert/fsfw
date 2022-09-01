#ifndef FSFW_TMTCSERVICES_TMHELPERS_H_
#define FSFW_TMTCSERVICES_TMHELPERS_H_

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

// I'd prefer to use tm, but there have been nameclashes with the tm struct
namespace telemetry {

class DataWithObjectIdPrefix : public SerializeIF {
 public:
  DataWithObjectIdPrefix(object_id_t objectId, const uint8_t* srcData, size_t srcDataLen)
      : objectId(objectId) {
    dataWrapper.type = util::DataTypes::RAW;
    dataWrapper.dataUnion.raw.data = srcData;
    dataWrapper.dataUnion.raw.len = srcDataLen;
  }

  DataWithObjectIdPrefix(object_id_t objectId, SerializeIF& serializable) : objectId(objectId) {
    dataWrapper.type = util::DataTypes::SERIALIZABLE;
    dataWrapper.dataUnion.serializable = &serializable;
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    if (*size + getSerializedSize() > maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    if (dataWrapper.type != util::DataTypes::RAW) {
      if ((dataWrapper.dataUnion.raw.data == nullptr) and (dataWrapper.dataUnion.raw.len > 0)) {
        return returnvalue::FAILED;
      }
    } else if (dataWrapper.type == util::DataTypes::SERIALIZABLE) {
      if (dataWrapper.dataUnion.serializable == nullptr) {
        return returnvalue::FAILED;
      }
    }
    ReturnValue_t result =
        SerializeAdapter::serialize(&objectId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (dataWrapper.type != util::DataTypes::RAW) {
      std::memcpy(*buffer, dataWrapper.dataUnion.raw.data, dataWrapper.dataUnion.raw.len);
      *buffer += dataWrapper.dataUnion.raw.len;
      *size += dataWrapper.dataUnion.raw.len;
    } else {
      return dataWrapper.dataUnion.serializable->serialize(buffer, size, maxSize, streamEndianness);
    }
    return returnvalue::OK;
  }

  [[nodiscard]] size_t getSerializedSize() const override {
    return sizeof(objectId) + dataWrapper.getLength();
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    // As long as there is no way to know how long the expected data will be, this function
    // does not make sense
    return returnvalue::FAILED;
  }

 private:
  object_id_t objectId;
  util::DataWrapper dataWrapper{};
};

}  // namespace telemetry

#endif  // FSFW_TMTCSERVICES_TMHELPERS_H_
