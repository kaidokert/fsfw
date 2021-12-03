#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_

#include "fsfw/cfdp/pdu/HeaderSerializer.h"

class FileDirectiveSerializer: public HeaderSerializer {
public:
    FileDirectiveSerializer(PduConfig& pduConf, cfdp::FileDirectives directiveCode,
            size_t directiveParamFieldLen);

    /**
     * This only returns the size of the PDU header + 1 for the directive code octet.
     * Use FileDirectiveSerializer::getWholePduSize to get the full packet length, assuming
     * the length fields was set correctly
     * @return
     */
    size_t getSerializedSize() const override;

    ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
            Endianness streamEndianness) const override;

    void setDirectiveDataFieldLen(size_t len);
private:
    cfdp::FileDirectives directiveCode = cfdp::FileDirectives::INVALID_DIRECTIVE;

};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_ */
