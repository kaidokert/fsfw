#ifndef FSFW_SRC_FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"
#include "fsfw/cfdp/definitions.h"
#include "fsfw/cfdp/FileSize.h"
#include "NakInfo.h"

#include <vector>

class NakPduSerializer: public FileDirectiveSerializer {
public:

    /**
     *
     * @param PduConf
     * @param startOfScope
     * @param endOfScope
     * @param [in] segmentRequests Pointer to the start of a list of segment requests
     * @param segmentRequestLen Length of the segment request list to be serialized
     */
    NakPduSerializer(PduConfig& PduConf, NakInfo& nakInfo);

    size_t getSerializedSize() const override;

    ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
            Endianness streamEndianness) const override;

    /**
     * If you change the info struct, you might need to update the directive field length
     * manually
     */
    void updateDirectiveFieldLen();
private:
    NakInfo& nakInfo;

};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_ */
