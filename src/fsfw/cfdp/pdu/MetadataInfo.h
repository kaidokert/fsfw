#ifndef FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_
#define FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_

#include "fsfw/cfdp/tlv/Tlv.h"
#include "fsfw/cfdp/tlv/Lv.h"
#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/definitions.h"

class MetadataInfo {
public:
    MetadataInfo(bool closureRequested, cfdp::ChecksumType checksumType, cfdp::FileSize& fileSize,
            cfdp::Lv& sourceFileName, cfdp::Lv& destFileName);

    size_t getSerializedSize(bool fssLarge = false);

    void setOptionsArray(cfdp::Tlv** optionsArray, size_t* optionsLen, size_t* maxOptionsLen);
    cfdp::ChecksumType getChecksumType() const;
    void setChecksumType(cfdp::ChecksumType checksumType);
    bool isClosureRequested() const;
    void setClosureRequested(bool closureRequested = false);

    void setDestFileName(cfdp::Lv& destFileName);
    void setSourceFileName(cfdp::Lv& sourceFileName);

    cfdp::Lv& getDestFileName();
    cfdp::Lv& getSourceFileName();
    cfdp::FileSize& getFileSize();

    bool hasOptions() const;
    bool canHoldOptions() const;
    ReturnValue_t getOptions(cfdp::Tlv*** optionsArray, size_t* optionsLen, size_t* maxOptsLen);
    void setOptionsLen(size_t optionsLen);
    size_t getOptionsLen() const;
    void setMaxOptionsLen(size_t maxOptionsLen);
    size_t getMaxOptionsLen() const;

private:
    bool closureRequested = false;
    cfdp::ChecksumType checksumType;
    cfdp::FileSize& fileSize;
    cfdp::Lv& sourceFileName;
    cfdp::Lv& destFileName;

    cfdp::Tlv** optionsArray = nullptr;
    size_t optionsLen = 0;
    size_t maxOptionsLen = 0;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_METADATAINFO_H_ */
