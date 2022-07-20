#ifndef FSFW_TMTCSERVICES_SENDANDSTOREHELPER_H
#define FSFW_TMTCSERVICES_SENDANDSTOREHELPER_H

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

namespace tm {

ReturnValue_t storeAndSendTmPacket(TmStoreHelper& storeHelper, TmSendHelper& sendHelper) {
  storeHelper.addPacketToStore();
  ReturnValue_t result = sendHelper.sendPacket(storeHelper.getCurrentAddr());
  if (result != HasReturnvaluesIF::RETURN_OK) {
    storeHelper.deletePacket();
  }
  return result;
}

class SourceDataWithObjectIdPrefix : public SerializeIF {
 public:
  SourceDataWithObjectIdPrefix(object_id_t objectId, const uint8_t* srcData, size_t srcDataLen)
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

}  // namespace tm

#endif  // FSFW_TMTCSERVICES_SENDANDSTOREHELPER_H
