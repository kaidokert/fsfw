#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDATAINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDATAINFO_H_

#include <fsfw/cfdp/FileSize.h>
#include <fsfw/cfdp/definitions.h>

class FileDataInfo {
 public:
  explicit FileDataInfo(cfdp::FileSize& offset);
  FileDataInfo(cfdp::FileSize& offset, const uint8_t* fileData, size_t fileSize);

  [[nodiscard]] size_t getSerializedSize(bool largeFile = false) const;

  cfdp::FileSize& getOffset();
  const uint8_t* getFileData(size_t* fileSize = nullptr) const;
  void setFileData(const uint8_t* fileData, size_t fileSize);

  [[nodiscard]] cfdp::SegmentMetadataFlag getSegmentMetadataFlag() const;
  [[nodiscard]] cfdp::SegmentationControl getSegmentationControl() const;
  [[nodiscard]] cfdp::RecordContinuationState getRecordContinuationState() const;
  void setRecordContinuationState(cfdp::RecordContinuationState recContState);
  void setSegmentationControl(cfdp::SegmentationControl segCtrl);

  [[nodiscard]] size_t getSegmentMetadataLen() const;
  void setSegmentMetadataLen(size_t len);
  void setSegmentMetadata(const uint8_t* ptr);
  [[nodiscard]] bool hasSegmentMetadata() const;
  void setSegmentMetadataFlag(bool enable);
  ReturnValue_t addSegmentMetadataInfo(cfdp::RecordContinuationState recContState,
                                       const uint8_t* segmentMetadata, size_t segmentMetadataLen);
  const uint8_t* getSegmentMetadata(size_t* segmentMetadataLen = nullptr);

 private:
  cfdp::SegmentMetadataFlag segmentMetadataFlag = cfdp::SegmentMetadataFlag::NOT_PRESENT;
  cfdp::SegmentationControl segCtrl = cfdp::SegmentationControl::NO_RECORD_BOUNDARIES_PRESERVATION;
  cfdp::FileSize& offset;
  const uint8_t* fileData = nullptr;
  size_t fileSize = 0;
  cfdp::RecordContinuationState recContState = cfdp::RecordContinuationState::NO_START_NO_END;
  size_t segmentMetadataLen = 0;
  const uint8_t* segmentMetadata = nullptr;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDATAINFO_H_ */
