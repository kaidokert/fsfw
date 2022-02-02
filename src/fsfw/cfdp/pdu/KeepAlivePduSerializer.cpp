#include "KeepAlivePduSerializer.h"

KeepAlivePduSerializer::KeepAlivePduSerializer(PduConfig &conf, cfdp::FileSize& progress):
        FileDirectiveSerializer(conf, cfdp::FileDirectives::KEEP_ALIVE, 4), progress(progress) {
    updateDirectiveFieldLen();
}

size_t KeepAlivePduSerializer::getSerializedSize() const {
    return FileDirectiveSerializer::getWholePduSize();
}

void KeepAlivePduSerializer::updateDirectiveFieldLen() {
    if(this->getLargeFileFlag()) {
        this->setDirectiveDataFieldLen(8);
    }
}

ReturnValue_t KeepAlivePduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
        Endianness streamEndianness) const {
    ReturnValue_t result = FileDirectiveSerializer::serialize(buffer, size, maxSize,
            streamEndianness);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return progress.serialize(this->getLargeFileFlag(), buffer, size, maxSize, streamEndianness);
}
