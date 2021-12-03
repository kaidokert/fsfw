#ifndef FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUSERIALIZER_H_

#include "fsfw/cfdp/pdu/MetadataInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"

class MetadataPduSerializer: public FileDirectiveSerializer {
public:
    MetadataPduSerializer(PduConfig &conf, MetadataInfo& info);

    void updateDirectiveFieldLen();

    size_t getSerializedSize() const override;

    ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
            Endianness streamEndianness) const override;
private:
    MetadataInfo& info;
};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_METADATAPDUSERIALIZER_H_ */
