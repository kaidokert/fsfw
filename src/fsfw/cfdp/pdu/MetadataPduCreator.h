#ifndef FSFW_CFDP_PDU_METADATAPDUCREATOR_H_
#define FSFW_CFDP_PDU_METADATAPDUCREATOR_H_

#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"
#include "fsfw/cfdp/pdu/MetadataInfo.h"

class MetadataPduCreator : public FileDirectiveCreator {
 public:
  MetadataPduCreator(PduConfig& conf, MetadataInfo& info);

  void updateDirectiveFieldLen();

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;
  using FileDirectiveCreator::serialize;

 private:
  MetadataInfo& info;
};

#endif /* FSFW_CFDP_PDU_METADATAPDUCREATOR_H_ */
