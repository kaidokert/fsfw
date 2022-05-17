#ifndef FSFW_SRC_FSFW_CFDP_FILESTORERESPONSETLV_H_
#define FSFW_SRC_FSFW_CFDP_FILESTORERESPONSETLV_H_

#include "Lv.h"
#include "TlvIF.h"
#include "fsfw/cfdp/tlv/FilestoreTlvBase.h"
#include "fsfw/cfdp/tlv/Tlv.h"

class FilestoreResponseTlv : public cfdp::FilestoreTlvBase {
 public:
  FilestoreResponseTlv(cfdp::Lv& firstFileName, cfdp::Lv* fsMsg);

  FilestoreResponseTlv(cfdp::FilestoreActionCode actionCode, uint8_t statusCode,
                       cfdp::Lv& firstFileName, cfdp::Lv* fsMsg);

  uint8_t getStatusCode() const;
  void setSecondFileName(cfdp::Lv* secondFileName);
  void setFilestoreMessage(cfdp::Lv* filestoreMsg);

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  /**
   * Deserialize a filestore response from a raw TLV object
   * @param tlv
   * @param endianness
   * @return
   */
  ReturnValue_t deSerialize(const cfdp::Tlv& tlv, Endianness endianness);

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;

  uint8_t getLengthField() const override;
  cfdp::TlvTypes getType() const override;

 private:
  uint8_t statusCode;
  cfdp::Lv* secondFileName = nullptr;
  cfdp::Lv* filestoreMsg = nullptr;

  ReturnValue_t deSerializeFromValue(const uint8_t** buffer, size_t* size,
                                     Endianness streamEndianness);
};

#endif /* FSFW_SRC_FSFW_CFDP_FILESTORERESPONSETLV_H_ */
