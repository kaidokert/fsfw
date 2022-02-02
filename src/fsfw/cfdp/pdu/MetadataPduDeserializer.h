#ifndef FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"
#include "fsfw/cfdp/pdu/MetadataInfo.h"

class MetadataPduDeserializer: public FileDirectiveDeserializer {
public:
    MetadataPduDeserializer(const uint8_t* pduBuf, size_t maxSize, MetadataInfo& info);

    ReturnValue_t parseData() override;
private:
    MetadataInfo& info;
};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUDESERIALIZER_H_ */
