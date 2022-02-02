#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"
#include "fsfw/cfdp/tlv/EntityIdTlv.h"
#include "EofInfo.h"

class EofPduSerializer: public FileDirectiveSerializer {
public:
    EofPduSerializer(PduConfig &conf, EofInfo& info);

    size_t getSerializedSize() const override;

    ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
            Endianness streamEndianness) const override;
private:
    EofInfo& info;
};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_ */
