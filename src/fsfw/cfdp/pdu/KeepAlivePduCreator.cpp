#include "KeepAlivePduCreator.h"

KeepAlivePduCreator::KeepAlivePduCreator(PduConfig &conf, cfdp::FileSize &progress)
    : FileDirectiveCreator(conf, cfdp::FileDirective::KEEP_ALIVE, 4), progress(progress) {
  updateDirectiveFieldLen();
}

size_t KeepAlivePduCreator::getSerializedSize() const {
  return FileDirectiveCreator::getWholePduSize();
}

void KeepAlivePduCreator::updateDirectiveFieldLen() {
  if (this->getLargeFileFlag()) {
    this->setDirectiveDataFieldLen(8);
  }
}

ReturnValue_t KeepAlivePduCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             Endianness streamEndianness) const {
  ReturnValue_t result = FileDirectiveCreator::serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return progress.serialize(this->getLargeFileFlag(), buffer, size, maxSize, streamEndianness);
}
