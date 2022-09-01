#include "fsfw/devicehandlers/DeviceTmReportingWrapper.h"

#include "fsfw/serialize/SerializeAdapter.h"

DeviceTmReportingWrapper::DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId,
                                                   const SerializeIF& data)
    : objectId(objectId), actionId(actionId), data(data) {}

DeviceTmReportingWrapper::~DeviceTmReportingWrapper() = default;

ReturnValue_t DeviceTmReportingWrapper::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                                  Endianness streamEndianness) const {
  ReturnValue_t result =
      SerializeAdapter::serialize(&objectId, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = SerializeAdapter::serialize(&actionId, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return data.serialize(buffer, size, maxSize, streamEndianness);
}

size_t DeviceTmReportingWrapper::getSerializedSize() const {
  return sizeof(objectId) + sizeof(ActionId_t) + data.getSerializedSize();
}

ReturnValue_t DeviceTmReportingWrapper::deSerialize(const uint8_t** buffer, size_t* size,
                                                    Endianness streamEndianness) {
  return returnvalue::FAILED;
}
