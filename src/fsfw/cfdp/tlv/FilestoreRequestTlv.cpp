#include "fsfw/cfdp/tlv/FilestoreRequestTlv.h"

#include "fsfw/FSFW.h"

FilestoreRequestTlv::FilestoreRequestTlv(cfdp::FilestoreActionCode actionCode,
                                         cfdp::StringLv &firstFileName)
    : FilestoreTlvBase(actionCode, firstFileName) {}

FilestoreRequestTlv::FilestoreRequestTlv(cfdp::StringLv &firstFileName)
    : FilestoreTlvBase(cfdp::FilestoreActionCode::INVALID, firstFileName) {}

void FilestoreRequestTlv::setSecondFileName(cfdp::Lv *secondFileName) {
  this->secondFileName = secondFileName;
}

ReturnValue_t FilestoreRequestTlv::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             Endianness streamEndianness) const {
  ReturnValue_t result = commonSerialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = firstFileName.serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (requiresSecondFileName()) {
    if (secondFileName == nullptr) {
      secondFileNameMissing();
      return cfdp::FILESTORE_REQUIRES_SECOND_FILE;
    }
    secondFileName->serialize(buffer, size, maxSize, streamEndianness);
  }
  return returnvalue::OK;
}

ReturnValue_t FilestoreRequestTlv::deSerialize(const uint8_t **buffer, size_t *size,
                                               Endianness streamEndianness) {
  ReturnValue_t result = commonDeserialize(buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return deSerializeFromValue(buffer, size, streamEndianness);
}

ReturnValue_t FilestoreRequestTlv::deSerialize(cfdp::Tlv &tlv, SerializeIF::Endianness endianness) {
  const uint8_t *ptr = tlv.getValue();
  size_t remSz = tlv.getSerializedSize();

  return deSerializeFromValue(&ptr, &remSz, endianness);
}

uint8_t FilestoreRequestTlv::getLengthField() const {
  size_t secondFileNameLen = 0;
  if (secondFileName != nullptr and requiresSecondFileName()) {
    secondFileNameLen = secondFileName->getSerializedSize();
  }
  return 1 + firstFileName.getSerializedSize() + secondFileNameLen;
}

ReturnValue_t FilestoreRequestTlv::deSerializeFromValue(const uint8_t **buffer, size_t *size,
                                                        Endianness streamEndianness) {
  this->actionCode = static_cast<cfdp::FilestoreActionCode>((**buffer >> 4) & 0x0f);
  *buffer += 1;
  *size -= 1;
  ReturnValue_t result = firstFileName.deSerialize(buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (requiresSecondFileName()) {
    if (secondFileName == nullptr) {
      secondFileNameMissing();
      return returnvalue::FAILED;
    }
    result = secondFileName->deSerialize(buffer, size, streamEndianness);
  }
  return result;
}

cfdp::TlvType FilestoreRequestTlv::getType() const { return cfdp::TlvType::FILESTORE_REQUEST; }
