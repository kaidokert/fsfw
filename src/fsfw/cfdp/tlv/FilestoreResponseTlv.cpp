#include "FilestoreResponseTlv.h"

FilestoreResponseTlv::FilestoreResponseTlv(cfdp::FilestoreActionCode actionCode, uint8_t statusCode,
                                           cfdp::StringLv &firstFileName, cfdp::Lv *fsMsg)
    : FilestoreTlvBase(actionCode, firstFileName), statusCode(statusCode), filestoreMsg(fsMsg) {}

FilestoreResponseTlv::FilestoreResponseTlv(cfdp::StringLv &firstFileName, cfdp::Lv *fsMsg)
    : FilestoreTlvBase(firstFileName), statusCode(0), filestoreMsg(fsMsg) {}

uint8_t FilestoreResponseTlv::getLengthField() const {
  size_t optFieldsLen = 0;
  if (secondFileName != nullptr) {
    optFieldsLen += secondFileName->getSerializedSize();
  }
  if (filestoreMsg != nullptr) {
    optFieldsLen += filestoreMsg->getSerializedSize();
  } else {
    optFieldsLen += 1;
  }
  return 1 + firstFileName.getSerializedSize() + optFieldsLen;
}

void FilestoreResponseTlv::setSecondFileName(cfdp::StringLv *secondFileName_) {
  this->secondFileName = secondFileName_;
}

void FilestoreResponseTlv::setFilestoreMessage(cfdp::Lv *filestoreMsg_) {
  this->filestoreMsg = filestoreMsg_;
}

ReturnValue_t FilestoreResponseTlv::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                              Endianness streamEndianness) const {
  ReturnValue_t result =
      commonSerialize(buffer, size, maxSize, streamEndianness, true, this->statusCode);
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
  }
  if (filestoreMsg != nullptr) {
    result = filestoreMsg->serialize(buffer, size, maxSize, streamEndianness);
  } else {
    if (*size == maxSize) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    **buffer = 0;
    *buffer += 1;
    *size += 1;
  }
  return result;
}

ReturnValue_t FilestoreResponseTlv::deSerialize(const uint8_t **buffer, size_t *size,
                                                Endianness streamEndianness) {
  ReturnValue_t result = commonDeserialize(buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return deSerializeFromValue(buffer, size, streamEndianness);
}

ReturnValue_t FilestoreResponseTlv::deSerializeFromValue(const uint8_t **buffer, size_t *size,
                                                         Endianness streamEndianness) {
  // The common function above checks whether at least one byte is remaining
  this->actionCode = static_cast<cfdp::FilestoreActionCode>((**buffer >> 4) & 0x0f);
  this->statusCode = **buffer & 0x0f;
  *buffer += 1;
  *size -= 1;
  ReturnValue_t result = firstFileName.deSerialize(buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (requiresSecondFileName()) {
    if (secondFileName == nullptr) {
      return cfdp::FILESTORE_REQUIRES_SECOND_FILE;
    }
    result = secondFileName->deSerialize(buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  // If the filestore message is not empty, the deserialization is only considered successfully
  // if the filestore message can be parsed. Also, if data follows after the FS response,
  // the FS msg needs to be set as well.
  if (*size == 0 or (*size > 1 and filestoreMsg == nullptr)) {
    // Filestore message missing or can't parse it
    return cfdp::FILESTORE_RESPONSE_CANT_PARSE_FS_MESSAGE;
  }
  if (filestoreMsg == nullptr) {
    *size -= 1;
    *buffer += 1;
    // Ignore empty filestore message
    return returnvalue::OK;
  }
  return filestoreMsg->deSerialize(buffer, size, streamEndianness);
}

ReturnValue_t FilestoreResponseTlv::deSerialize(const cfdp::Tlv &tlv, Endianness endianness) {
  const uint8_t *ptr = tlv.getValue();
  size_t remSz = tlv.getSerializedSize();

  return deSerializeFromValue(&ptr, &remSz, endianness);
}

uint8_t FilestoreResponseTlv::getStatusCode() const { return statusCode; }

cfdp::TlvType FilestoreResponseTlv::getType() const { return cfdp::TlvType::FILESTORE_RESPONSE; }
