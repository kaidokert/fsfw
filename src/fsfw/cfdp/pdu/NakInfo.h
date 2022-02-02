#ifndef FSFW_SRC_FSFW_CFDP_PDU_NAKINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_NAKINFO_H_

#include <utility>

#include "fsfw/cfdp/FileSize.h"

class NakInfo {
 public:
  using SegmentRequest = std::pair<cfdp::FileSize, cfdp::FileSize>;

  NakInfo(cfdp::FileSize startOfScope, cfdp::FileSize endOfScope);

  void setSegmentRequests(SegmentRequest* segmentRequests, size_t* segmentRequestLen,
                          size_t* maxSegmentRequestLen);

  size_t getSerializedSize(bool fssLarge = false);

  cfdp::FileSize& getStartOfScope();
  cfdp::FileSize& getEndOfScope();

  bool hasSegmentRequests() const;
  bool canHoldSegmentRequests() const;
  void setMaxSegmentRequestLen(size_t maxSize);
  bool getSegmentRequests(SegmentRequest** segmentRequestPtr, size_t* segmentRequestLen,
                          size_t* maxSegmentRequestsLen);
  size_t getSegmentRequestsLen() const;
  size_t getSegmentRequestsMaxLen() const;

  //! This functions is more relevant for deserializers
  void setSegmentRequestLen(size_t readLen);

 private:
  cfdp::FileSize startOfScope;
  cfdp::FileSize endOfScope;
  SegmentRequest* segmentRequests = nullptr;
  size_t segmentRequestsLen = 0;
  size_t maxSegmentRequestsLen = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_NAKINFO_H_ */
