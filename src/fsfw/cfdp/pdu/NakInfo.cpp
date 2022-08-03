#include "NakInfo.h"

NakInfo::NakInfo(cfdp::FileSize startOfScope, cfdp::FileSize endOfScope)
    : startOfScope(startOfScope), endOfScope(endOfScope) {}

size_t NakInfo::getSerializedSize(bool fssLarge) {
  size_t size = 8;
  if (fssLarge) {
    size += 8;
  }
  if (hasSegmentRequests()) {
    if (fssLarge) {
      size += segmentRequestsLen * 16;
    } else {
      size += segmentRequestsLen * 8;
    }
  }
  return size;
}

bool NakInfo::hasSegmentRequests() const {
  if (this->segmentRequests != nullptr and segmentRequestsLen > 0) {
    return true;
  }
  return false;
}

bool NakInfo::canHoldSegmentRequests() const {
  if (segmentRequests != nullptr and maxSegmentRequestsLen > 0) {
    return true;
  }
  return false;
}

bool NakInfo::getSegmentRequests(SegmentRequest** segmentRequestPtr, size_t* segmentRequestLen,
                                 size_t* maxSegmentRequestsLen) {
  if (this->segmentRequests != nullptr) {
    *segmentRequestPtr = this->segmentRequests;
  }
  if (segmentRequestLen != nullptr) {
    *segmentRequestLen = this->segmentRequestsLen;
  }
  if (maxSegmentRequestsLen != nullptr) {
    *maxSegmentRequestsLen = this->maxSegmentRequestsLen;
  }
  return true;
}

void NakInfo::setSegmentRequests(SegmentRequest* segmentRequests, size_t* segmentRequestLen,
                                 size_t* maxSegmentRequestLen) {
  this->segmentRequests = segmentRequests;
  if (segmentRequestLen != nullptr) {
    this->segmentRequestsLen = *segmentRequestLen;
  }
  if (maxSegmentRequestLen != nullptr) {
    this->maxSegmentRequestsLen = *maxSegmentRequestLen;
  }
}

cfdp::FileSize& NakInfo::getStartOfScope() { return startOfScope; }

cfdp::FileSize& NakInfo::getEndOfScope() { return endOfScope; }

size_t NakInfo::getSegmentRequestsLen() const { return segmentRequestsLen; }

size_t NakInfo::getSegmentRequestsMaxLen() const { return maxSegmentRequestsLen; }

void NakInfo::setSegmentRequestLen(size_t readLen) { this->segmentRequestsLen = readLen; }

void NakInfo::setMaxSegmentRequestLen(size_t maxSize) { this->maxSegmentRequestsLen = maxSize; }
