#ifndef FSFW_TMTCSERVICES_TMHELPERS_H_
#define FSFW_TMTCSERVICES_TMHELPERS_H_

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

// I'd prefer to use tm, but there have been nameclashes with the tm struct
namespace telemetry {

class DataWithObjectIdPrefix : public SerializeIF {
 public:
  DataWithObjectIdPrefix(object_id_t objectId, const uint8_t* srcData, size_t srcDataLen)
      : objectId(objectId), bufAdapter(srcData, srcDataLen), userData(&bufAdapter) {}

  DataWithObjectIdPrefix(object_id_t objectId, const SerializeIF& serializable)
      : objectId(objectId), userData(&serializable) {}

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    if (*size + getSerializedSize() > maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    ReturnValue_t result =
        SerializeAdapter::serialize(&objectId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (userData != nullptr) {
      return userData->serialize(buffer, size, maxSize, streamEndianness);
    }
    return returnvalue::OK;
  }

  [[nodiscard]] size_t getSerializedSize() const override {
    size_t len = 0;
    if (userData != nullptr) {
      len += userData->getSerializedSize();
    }
    return len;
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    // As long as there is no way to know how long the expected data will be, this function
    // does not make sense
    return returnvalue::FAILED;
  }

 private:
  object_id_t objectId;
  SerialBufferAdapter<uint8_t> bufAdapter;
  const SerializeIF* userData = nullptr;
};

}  // namespace telemetry

#endif  // FSFW_TMTCSERVICES_TMHELPERS_H_
