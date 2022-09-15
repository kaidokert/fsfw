#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_

#include "../definitions.h"
#include "fsfw/cfdp/pdu/PduHeaderReader.h"

/**
 * @brief   This class is used to deserialize a PDU file directive header from raw memory.
 * @details
 * Base class for other file directives.
 * This is a zero-copy implementation and #parseData needs to be called to ensure the data is
 * valid.
 */
class FileDirectiveReader : public PduHeaderReader {
 public:
  FileDirectiveReader(const uint8_t* pduBuf, size_t maxSize);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   */
  ReturnValue_t parseData() override;
  [[nodiscard]] size_t getHeaderSize() const override;

  [[nodiscard]] cfdp::FileDirective getFileDirective() const;

  void setEndianness(SerializeIF::Endianness endianness);
  [[nodiscard]] SerializeIF::Endianness getEndianness() const;
  static bool checkFileDirective(uint8_t rawByte);

 protected:
 private:
  void setFileDirective(cfdp::FileDirective fileDirective);
  cfdp::FileDirective fileDirective = cfdp::FileDirective::INVALID_DIRECTIVE;
  SerializeIF::Endianness endianness = SerializeIF::Endianness::NETWORK;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_ */
