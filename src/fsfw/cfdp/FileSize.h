#ifndef FSFW_SRC_FSFW_CFDP_FILESIZE_H_
#define FSFW_SRC_FSFW_CFDP_FILESIZE_H_

#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serialize/SerializeIF.h"

namespace cfdp {

struct FileSize : public SerializeIF {
 public:
  FileSize() : largeFile(false){};

  FileSize(uint64_t fileSize, bool isLarge = false) { setFileSize(fileSize, isLarge); };

  ReturnValue_t serialize(bool isLarge, uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) {
    this->largeFile = isLarge;
    return serialize(buffer, size, maxSize, streamEndianness);
  }

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    if (not largeFile) {
      uint32_t fileSizeTyped = fileSize;
      return SerializeAdapter::serialize(&fileSizeTyped, buffer, size, maxSize, streamEndianness);
    }
    return SerializeAdapter::serialize(&fileSize, buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const override {
    if (largeFile) {
      return 8;
    } else {
      return 4;
    }
  }

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    if (largeFile) {
      return SerializeAdapter::deSerialize(&size, buffer, size, streamEndianness);
    } else {
      uint32_t sizeTmp = 0;
      ReturnValue_t result =
          SerializeAdapter::deSerialize(&sizeTmp, buffer, size, streamEndianness);
      if (result == HasReturnvaluesIF::RETURN_OK) {
        fileSize = sizeTmp;
      }
      return result;
    }
  }

  ReturnValue_t setFileSize(uint64_t fileSize, bool largeFile) {
    if (not largeFile and fileSize > UINT32_MAX) {
      // TODO: emit warning here
      return HasReturnvaluesIF::RETURN_FAILED;
    }
    this->fileSize = fileSize;
    this->largeFile = largeFile;
    return HasReturnvaluesIF::RETURN_OK;
  }

  bool isLargeFile() const { return largeFile; }
  uint64_t getSize(bool *largeFile = nullptr) const {
    if (largeFile != nullptr) {
      *largeFile = this->largeFile;
    }
    return fileSize;
  }

 private:
  uint64_t fileSize = 0;
  bool largeFile = false;
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_FILESIZE_H_ */
