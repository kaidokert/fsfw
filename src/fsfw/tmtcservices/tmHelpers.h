#ifndef FSFW_TMTCSERVICES_TMHELPERS_H_
#define FSFW_TMTCSERVICES_TMHELPERS_H_

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

// I'd prefer to use tm, but there have been nameclashes with the tm struct
namespace telemetry {

ReturnValue_t storeAndSendTmPacket(TmStoreHelper& storeHelper, TmSendHelper& sendHelper,
                                   bool delOnFailure = true);

class DataWithObjectIdPrefix : public SerializeIF {
 public:
  DataWithObjectIdPrefix(object_id_t objectId, const uint8_t* srcData, size_t srcDataLen)
      : objectId(objectId), srcData(srcData), srcDataLen(srcDataLen) {}
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    if (*size + getSerializedSize() > maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    ReturnValue_t result =
        SerializeAdapter::serialize(&objectId, buffer, size, maxSize, streamEndianness);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      return result;
    }
    std::memcpy(*buffer, srcData, srcDataLen);
    *buffer += srcDataLen;
    *size += srcDataLen;
    return HasReturnvaluesIF::RETURN_OK;
  }

  [[nodiscard]] size_t getSerializedSize() const override { return sizeof(objectId) + srcDataLen; }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    // TODO: Implement
    return HasReturnvaluesIF::RETURN_FAILED;
  }

 private:
  object_id_t objectId;
  const uint8_t* srcData;
  size_t srcDataLen;
};

}  // namespace telemetry

#endif  // FSFW_TMTCSERVICES_TMHELPERS_H_
