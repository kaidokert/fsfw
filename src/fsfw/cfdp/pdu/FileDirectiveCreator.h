#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_

#include "fsfw/cfdp/pdu/HeaderCreator.h"

class FileDirectiveCreator : public HeaderCreator {
 public:
  FileDirectiveCreator(PduConfig& pduConf, cfdp::FileDirective directiveCode,
                       size_t directiveParamFieldLen);

  [[nodiscard]] cfdp::FileDirective getDirectiveCode() const;

  /**
   * This only returns the size of the PDU header + 1 for the directive code octet.
   * Use FileDirectiveCreator::getWholePduSize to get the full packet length, assuming
   * the length fields was set correctly
   * @return
   */
  [[nodiscard]] size_t getSerializedSize() const override;

  [[nodiscard]] ReturnValue_t serialize(uint8_t* buffer, size_t& serLen, size_t maxSize) const {
    return SerializeIF::serialize(buffer, serLen, maxSize, SerializeIF::Endianness::NETWORK);
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  void setDirectiveDataFieldLen(size_t len);

 private:
  cfdp::FileDirective directiveCode = cfdp::FileDirective::INVALID_DIRECTIVE;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVESERIALIZER_H_ */
