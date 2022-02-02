#include "MetadataInfo.h"

MetadataInfo::MetadataInfo(bool closureRequested, cfdp::ChecksumType checksumType,
        cfdp::FileSize& fileSize, cfdp::Lv& sourceFileName, cfdp::Lv& destFileName):
        closureRequested(closureRequested), checksumType(checksumType), fileSize(fileSize),
        sourceFileName(sourceFileName), destFileName(destFileName) {
}

void MetadataInfo::setOptionsArray(cfdp::Tlv** optionsArray, size_t* optionsLen,
        size_t* maxOptionsLen) {
    this->optionsArray = optionsArray;
    if(maxOptionsLen != nullptr) {
        this->maxOptionsLen = *maxOptionsLen;
    }
    if(optionsLen != nullptr) {
        this->optionsLen = *optionsLen;
    }
}

cfdp::ChecksumType MetadataInfo::getChecksumType() const {
    return checksumType;
}

void MetadataInfo::setChecksumType(cfdp::ChecksumType checksumType) {
    this->checksumType = checksumType;
}

bool MetadataInfo::isClosureRequested() const {
    return closureRequested;
}

void MetadataInfo::setClosureRequested(bool closureRequested) {
    this->closureRequested = closureRequested;
}

cfdp::Lv& MetadataInfo::getDestFileName() {
    return destFileName;
}

cfdp::FileSize& MetadataInfo::getFileSize() {
    return fileSize;
}

ReturnValue_t MetadataInfo::getOptions(cfdp::Tlv*** optionsArray, size_t *optionsLen,
        size_t* maxOptsLen) {
    if(optionsArray == nullptr or this->optionsArray == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    *optionsArray = this->optionsArray;
    if(optionsLen != nullptr) {
        *optionsLen = this->optionsLen;
    }
    if(maxOptsLen != nullptr) {
        *maxOptsLen = this->maxOptionsLen;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

bool MetadataInfo::hasOptions() const {
    if (optionsArray != nullptr and optionsLen > 0) {
        return true;
    }
    return false;
}

bool MetadataInfo::canHoldOptions() const {
    if (optionsArray != nullptr and maxOptionsLen > 0) {
        return true;
    }
    return false;
}

size_t MetadataInfo::getSerializedSize(bool fssLarge) {
    // 1 byte + minimal FSS 4 bytes
    size_t size = 5;
    if(fssLarge) {
        size += 4;
    }
    size += sourceFileName.getSerializedSize();
    size += destFileName.getSerializedSize();
    if(hasOptions()) {
        for(size_t idx = 0; idx < optionsLen; idx++) {
            size += optionsArray[idx]->getSerializedSize();
        }
    }
    return size;
}

void MetadataInfo::setDestFileName(cfdp::Lv &destFileName) {
    this->destFileName = destFileName;
}

void MetadataInfo::setSourceFileName(cfdp::Lv &sourceFileName) {
    this->sourceFileName = sourceFileName;
}

size_t MetadataInfo::getMaxOptionsLen() const {
    return maxOptionsLen;
}

void MetadataInfo::setMaxOptionsLen(size_t maxOptionsLen) {
    this->maxOptionsLen = maxOptionsLen;
}

size_t MetadataInfo::getOptionsLen() const {
    return optionsLen;
}

void MetadataInfo::setOptionsLen(size_t optionsLen) {
    this->optionsLen = optionsLen;
}

cfdp::Lv& MetadataInfo::getSourceFileName() {
    return sourceFileName;
}
