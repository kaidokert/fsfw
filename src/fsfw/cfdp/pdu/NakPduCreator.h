#ifndef FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_
#define FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_

#include <vector>

#include "NakInfo.h"
#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/definitions.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"

class NakPduCreator : public FileDirectiveCreator {
 public:
  /**
   *
   * @param PduConf
   * @param startOfScope
   * @param endOfScope
   * @param [in] segmentRequests Pointer to the start of a list of segment requests
   * @param segmentRequestLen Length of the segment request list to be serialized
   */
  NakPduCreator(PduConfig& PduConf, NakInfo& nakInfo);

  [[nodiscard]] size_t getSerializedSize() const override;

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

#endif /* FSFW_CFDP_PDU_NAKPDUSERIALIZER_H_ */
