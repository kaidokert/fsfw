#ifndef FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTBASE_H_
#define FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTBASE_H_

#include <fsfw/cfdp/definitions.h>
#include <fsfw/cfdp/tlv/Lv.h>
#include <fsfw/cfdp/tlv/Tlv.h>
#include <fsfw/cfdp/tlv/TlvIF.h>
#include <fsfw/serialize/SerializeIF.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

#include <cstddef>
#include <cstdint>

#include "StringLv.h"
#include "fsfw/FSFW.h"

namespace cfdp {

enum FilestoreActionCode {
  CREATE_FILE = 0b0000,
  DELETE_FILE = 0b0001,
  // Second file name present
  RENAME_FILE = 0b0010,
  // Second file name present
  APPEND_FILE = 0b0011,
  // Second file name present
  REPLACE_FILE = 0b0100,
  CREATE_DIRECTORY = 0b0101,
  REMOVE_DIRECTORY = 0b0110,
  // Delete file if present
  DENY_FILE = 0b0111,
  // Remove directory if present
  DNEY_DIRECTORY = 0b1000,
  INVALID = 0b1111
};

// FSR = Filestore Response
static constexpr uint8_t FSR_SUCCESS = 0b0000;
static constexpr uint8_t FSR_NOT_PERFORMED = 0b1111;

static constexpr uint8_t FSR_CREATE_NOT_ALLOWED = 0b0001;

static constexpr uint8_t FSR_DEL_FILE_NOT_EXISTS = 0b0001;
static constexpr uint8_t FSR_DEL_NOT_ALLOWED = 0b0010;

static constexpr uint8_t FSR_RENAME_OLD_FILE_NOT_EXISTS = 0b0001;
static constexpr uint8_t FSR_RENAME_NEW_FILE_ALREADY_EXISTS = 0b0010;
static constexpr uint8_t FSR_RENAME_NOT_ALLOWED = 0b0011;

static constexpr uint8_t FSR_APPEND_FILE_1_NOT_EXISTS = 0b0001;
static constexpr uint8_t FSR_APPEND_FILE_2_NOT_EXISTS = 0b0010;
static constexpr uint8_t FSR_APPEND_NOT_ALLOWED = 0b0011;

static constexpr uint8_t FSR_REPLACE_FILE_1_NOT_EXISTS = 0b0001;
static constexpr uint8_t FSR_REPLACE_FILE_2_NOT_EXISTS = 0b0010;
static constexpr uint8_t FSR_REPLACE_REPLACE_NOT_ALLOWED = 0b0011;

static constexpr uint8_t FSR_CREATE_DIR_NOT_POSSIBLE = 0b0001;

static constexpr uint8_t FSR_DELDIR_NOT_EXISTS = 0b0001;
static constexpr uint8_t FSR_DELDIR_NOT_ALLOWED = 0b0010;

static constexpr uint8_t FSR_DENY_FILE_NOT_ALLOWED = 0b0010;

static constexpr uint8_t FSR_DENY_DIR_NOT_ALLOWED = 0b0010;

class FilestoreTlvBase : public TlvIF {
 public:
  explicit FilestoreTlvBase(cfdp::StringLv& firstFileName) : firstFileName(firstFileName){};
  FilestoreTlvBase(FilestoreActionCode actionCode, cfdp::StringLv& firstFileName)
      : actionCode(actionCode), firstFileName(firstFileName){};

  ReturnValue_t commonSerialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                Endianness streamEndianness, bool isResponse = false,
                                uint8_t responseStatusCode = 0) const {
    if (buffer == nullptr or size == nullptr) {
      return returnvalue::FAILED;
    }
    if (maxSize < 3) {
      return SerializeIF::BUFFER_TOO_SHORT;
    }
    **buffer = getType();
    *buffer += 1;
    *size += 1;
    **buffer = getLengthField();
    *buffer += 1;
    *size += 1;
    **buffer = this->actionCode << 4;
    if (isResponse) {
      **buffer |= responseStatusCode;
    }
    *buffer += 1;
    *size += 1;
    return returnvalue::OK;
  }

  ReturnValue_t commonDeserialize(const uint8_t** buffer, size_t* size,
                                  SerializeIF::Endianness streamEndianness) {
    if (buffer == nullptr or size == nullptr) {
      return returnvalue::FAILED;
    }
    if (*size < 3) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    auto type = static_cast<cfdp::TlvType>(**buffer);
    if (type != getType()) {
      return cfdp::INVALID_TLV_TYPE;
    }
    *size -= 1;
    *buffer += 1;

    size_t remainingLength = **buffer;
    *size -= 1;
    *buffer += 1;
    if (remainingLength == 0) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    return returnvalue::OK;
  }

  [[nodiscard]] bool requiresSecondFileName() const {
    using namespace cfdp;
    if (actionCode == FilestoreActionCode::RENAME_FILE or
        actionCode == FilestoreActionCode::APPEND_FILE or
        actionCode == FilestoreActionCode::REPLACE_FILE) {
      return true;
    }
    return false;
  }

  void secondFileNameMissing() const {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "FilestoreRequestTlv::deSerialize: Second file name required"
                    " but TLV pointer not set"
                 << std::endl;
#else
    sif::printWarning(
        "FilestoreRequestTlv::deSerialize: Second file name required"
        " but TLV pointer not set\n");
#endif
#endif
  }

  [[nodiscard]] FilestoreActionCode getActionCode() const { return actionCode; }

  void setActionCode(FilestoreActionCode actionCode_) { this->actionCode = actionCode_; }

  cfdp::Lv& getFirstFileName() { return firstFileName; }

  ReturnValue_t convertToTlv(cfdp::Tlv& tlv, uint8_t* buffer, size_t maxSize,
                             Endianness streamEndianness) {
    size_t serSize = 0;
    uint8_t* valueStart = buffer + 2;
    ReturnValue_t result = this->serialize(&buffer, &serSize, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    tlv.setValue(valueStart, serSize - 2);
    tlv.setType(getType());
    return result;
  }

  [[nodiscard]] size_t getSerializedSize() const override { return getLengthField() + 2; }

 protected:
  FilestoreActionCode actionCode = FilestoreActionCode::INVALID;
  cfdp::StringLv& firstFileName;
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTBASE_H_ */
