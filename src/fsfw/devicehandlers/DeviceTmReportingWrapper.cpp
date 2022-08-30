#include "fsfw/devicehandlers/DeviceTmReportingWrapper.h"

#include "fsfw/serialize/SerializeAdapter.h"

DeviceTmReportingWrapper::DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId,
                                                   util::DataWrapper data)
    : objectId(objectId), actionId(actionId), dataWrapper(data) {}

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
  if (dataWrapper.isNull()) {
    return returnvalue::FAILED;
  }
  if (dataWrapper.type == util::DataTypes::SERIALIZABLE) {
    return dataWrapper.dataUnion.serializable->serialize(buffer, size, maxSize, streamEndianness);
  } else if (dataWrapper.type == util::DataTypes::RAW) {
    if (*size + dataWrapper.dataUnion.raw.len > maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    std::memcpy(*buffer, dataWrapper.dataUnion.raw.data, dataWrapper.dataUnion.raw.len);
    *buffer += dataWrapper.dataUnion.raw.len;
    *size += dataWrapper.dataUnion.raw.len;
  }
  return returnvalue::OK;
}

size_t DeviceTmReportingWrapper::getSerializedSize() const {
  return sizeof(objectId) + sizeof(ActionId_t) + dataWrapper.getLength();
}

ReturnValue_t DeviceTmReportingWrapper::deSerialize(const uint8_t** buffer, size_t* size,
                                                    Endianness streamEndianness) {
  return returnvalue::FAILED;
}
