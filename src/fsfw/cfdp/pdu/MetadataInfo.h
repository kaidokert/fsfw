#ifndef FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_

#include <optional>

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/definitions.h"
#include "fsfw/cfdp/tlv/Lv.h"
#include "fsfw/cfdp/tlv/StringLv.h"
#include "fsfw/cfdp/tlv/Tlv.h"

class MetadataInfo {
 public:
  MetadataInfo(cfdp::FileSize& fileSize, cfdp::StringLv& sourceFileName,
               cfdp::StringLv& destFileName);
  MetadataInfo(bool closureRequested, cfdp::ChecksumType checksumType, cfdp::FileSize& fileSize,
               cfdp::StringLv& sourceFileName, cfdp::StringLv& destFileName);

  size_t getSerializedSize(bool fssLarge = false);

  void setOptionsArray(cfdp::Tlv** optionsArray, std::optional<size_t> optionsLen,
                       std::optional<size_t> maxOptionsLen);
  [[nodiscard]] cfdp::ChecksumType getChecksumType() const;
  void setChecksumType(cfdp::ChecksumType checksumType);
  [[nodiscard]] bool isClosureRequested() const;
  void setClosureRequested(bool closureRequested = false);

  void setDestFileName(cfdp::StringLv& destFileName);
  void setSourceFileName(cfdp::StringLv& sourceFileName);

  cfdp::StringLv& getDestFileName();
  cfdp::StringLv& getSourceFileName();
  cfdp::FileSize& getFileSize();

  [[nodiscard]] bool hasOptions() const;
  [[nodiscard]] bool canHoldOptions() const;
  ReturnValue_t getOptions(cfdp::Tlv*** optionsArray, size_t* optionsLen, size_t* maxOptsLen);
  void setOptionsLen(size_t optionsLen);
  [[nodiscard]] size_t getOptionsLen() const;
  void setMaxOptionsLen(size_t maxOptionsLen);
  [[nodiscard]] size_t getMaxOptionsLen() const;

 private:
  bool closureRequested = false;
  cfdp::ChecksumType checksumType = cfdp::ChecksumType::NULL_CHECKSUM;
  cfdp::FileSize& fileSize;
  cfdp::StringLv& sourceFileName;
  cfdp::StringLv& destFileName;

  cfdp::Tlv** optionsArray = nullptr;
  size_t optionsLen = 0;
  size_t maxOptionsLen = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_ */
