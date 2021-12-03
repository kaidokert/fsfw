#ifndef FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTTLV_H_
#define FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTTLV_H_

#include "fsfw/cfdp/tlv/FilestoreTlvBase.h"
#include "fsfw/cfdp/tlv/Tlv.h"
#include "TlvIF.h"
#include "Lv.h"
#include "../definitions.h"

class FilestoreRequestTlv:
        public cfdp::FilestoreTlvBase {
public:
    FilestoreRequestTlv(cfdp::FilestoreActionCode actionCode, cfdp::Lv& firstFileName);

    FilestoreRequestTlv(cfdp::Lv& firstFileName);

    void setSecondFileName(cfdp::Lv* secondFileName);

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override;

    /**
     * Deserialize a FS request from a raw TLV object
     * @param tlv
     * @param endianness
     * @return
     */
    ReturnValue_t deSerialize(cfdp::Tlv& tlv, Endianness endianness);

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override;

    uint8_t getLengthField() const override;
    cfdp::TlvTypes getType() const override;

private:
    cfdp::Lv* secondFileName = nullptr;

    ReturnValue_t deSerializeFromValue(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness);
};



#endif /* FSFW_SRC_FSFW_CFDP_FILESTOREREQUESTTLV_H_ */
