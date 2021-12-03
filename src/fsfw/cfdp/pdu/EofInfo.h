#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_

#include "fsfw/cfdp/tlv/EntityIdTlv.h"
#include "../definitions.h"
#include "../FileSize.h"

struct EofInfo {
public:
    EofInfo(EntityIdTlv* faultLoc = nullptr);
    EofInfo(cfdp::ConditionCode conditionCode, uint32_t checksum, cfdp::FileSize fileSize,
            EntityIdTlv* faultLoc = nullptr);

    size_t getSerializedSize(bool fssLarge = false);

    uint32_t getChecksum() const;
    cfdp::ConditionCode getConditionCode() const;

    EntityIdTlv* getFaultLoc() const;
    cfdp::FileSize& getFileSize();
    void setChecksum(uint32_t checksum);
    void setConditionCode(cfdp::ConditionCode conditionCode);
    void setFaultLoc(EntityIdTlv *faultLoc);
    ReturnValue_t setFileSize(size_t size, bool isLarge);
private:

    cfdp::ConditionCode conditionCode;
    uint32_t checksum;
    cfdp::FileSize fileSize;
    EntityIdTlv* faultLoc = nullptr;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFINFO_H_ */
