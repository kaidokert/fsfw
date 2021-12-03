#ifndef FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"
#include "fsfw/cfdp/pdu/FileDataSerializer.h"
#include "FinishedInfo.h"

class FinishPduSerializer: public FileDirectiveSerializer {
public:
    FinishPduSerializer(PduConfig& pduConf, FinishedInfo& finishInfo);

    void updateDirectiveFieldLen();

    size_t getSerializedSize() const override;

    ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
            Endianness streamEndianness) const override;
private:
    FinishedInfo& finishInfo;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_ */
