#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_

#include "../FileSize.h"
#include "../definitions.h"
#include "fsfw/cfdp/tlv/EntityIdTlv.h"

struct EofInfo {
 public:
  explicit EofInfo(EntityIdTlv* faultLoc = nullptr);
  EofInfo(cfdp::ConditionCode conditionCode, uint32_t checksum, cfdp::FileSize fileSize,
          EntityIdTlv* faultLoc = nullptr);

  size_t getSerializedSize(bool fssLarge = false);

  [[nodiscard]] uint32_t getChecksum() const;
  [[nodiscard]] cfdp::ConditionCode getConditionCode() const;

  [[nodiscard]] EntityIdTlv* getFaultLoc() const;
  cfdp::FileSize& getFileSize();
  void setChecksum(uint32_t checksum);
  void setConditionCode(cfdp::ConditionCode conditionCode);
  void setFaultLoc(EntityIdTlv* faultLoc);
  ReturnValue_t setFileSize(size_t size, bool isLarge);

 private:
  cfdp::ConditionCode conditionCode;
  uint32_t checksum;
  cfdp::FileSize fileSize;
  EntityIdTlv* faultLoc = nullptr;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_ */
