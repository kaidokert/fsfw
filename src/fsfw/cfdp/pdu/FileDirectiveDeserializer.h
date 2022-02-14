#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_

#include "../definitions.h"
#include "fsfw/cfdp/pdu/HeaderDeserializer.h"

/**
 * @brief   This class is used to deserialize a PDU file directive header from raw memory.
 * @details
 * Base class for other file directives.
 * This is a zero-copy implementation and #parseData needs to be called to ensure the data is
 * valid.
 */
class FileDirectiveDeserializer : public HeaderDeserializer {
 public:
  FileDirectiveDeserializer(const uint8_t* pduBuf, size_t maxSize);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   */
  virtual ReturnValue_t parseData();
  size_t getHeaderSize() const;

  cfdp::FileDirectives getFileDirective() const;

  void setEndianness(SerializeIF::Endianness endianness);
  SerializeIF::Endianness getEndianness() const;

 protected:
  bool checkFileDirective(uint8_t rawByte);

 private:
  void setFileDirective(cfdp::FileDirectives fileDirective);
  cfdp::FileDirectives fileDirective = cfdp::FileDirectives::INVALID_DIRECTIVE;
  SerializeIF::Endianness endianness = SerializeIF::Endianness::NETWORK;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDIRECTIVEDESERIALIZER_H_ */
