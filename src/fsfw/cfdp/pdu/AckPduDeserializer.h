#ifndef FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"
#include "AckInfo.h"

class AckPduDeserializer: public FileDirectiveDeserializer {
public:
    AckPduDeserializer(const uint8_t* pduBuf, size_t maxSize, AckInfo& info);

    /**
     *
     * @return
     *  - cfdp::INVALID_DIRECTIVE_FIELDS: Invalid fields
     */
    ReturnValue_t parseData();

private:
    bool checkAndSetCodes(uint8_t rawAckedByte, uint8_t rawAckedConditionCode);
    AckInfo& info;

};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_ */
