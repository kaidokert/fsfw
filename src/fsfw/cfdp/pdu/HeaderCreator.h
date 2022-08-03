#ifndef FSFW_SRC_FSFW_CFDP_PDU_HEADERSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_HEADERSERIALIZER_H_

#include "../definitions.h"
#include "PduConfig.h"
#include "PduHeaderIF.h"
#include "fsfw/serialize/SerializeIF.h"

class HeaderCreator : public SerializeIF, public PduHeaderIF {
 public:
  HeaderCreator(
      PduConfig& pduConf, cfdp::PduType pduType, size_t initPduDataFieldLen,
      cfdp::SegmentMetadataFlag segmentMetadataFlag = cfdp::SegmentMetadataFlag::NOT_PRESENT,
      cfdp::SegmentationControl segCtrl =
          cfdp::SegmentationControl::NO_RECORD_BOUNDARIES_PRESERVATION);

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  /**
   * This only returns the length of the serialized hader.
   * Use #getWholePduSize to get the length of the full packet, assuming that the PDU
   * data field length was not properly.
   * @return
   */
  size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;

  void setPduDataFieldLen(size_t pduDataFieldLen);
  void setPduType(cfdp::PduType pduType);
  void setSegmentMetadataFlag(cfdp::SegmentMetadataFlag);

  size_t getPduDataFieldLen() const override;
  size_t getWholePduSize() const override;

  cfdp::PduType getPduType() const override;
  cfdp::Direction getDirection() const override;
  cfdp::TransmissionModes getTransmissionMode() const override;
  bool getCrcFlag() const override;
  bool getLargeFileFlag() const override;
  cfdp::SegmentationControl getSegmentationControl() const override;
  cfdp::WidthInBytes getLenEntityIds() const override;
  cfdp::WidthInBytes getLenSeqNum() const override;
  cfdp::SegmentMetadataFlag getSegmentMetadataFlag() const override;
  bool hasSegmentMetadataFlag() const override;
  void setSegmentationControl(cfdp::SegmentationControl);

  void getSourceId(cfdp::EntityId& sourceId) const override;
  void getDestId(cfdp::EntityId& destId) const override;
  void getTransactionSeqNum(cfdp::TransactionSeqNum& seqNum) const override;

 private:
  cfdp::PduType pduType;
  cfdp::SegmentMetadataFlag segmentMetadataFlag;
  cfdp::SegmentationControl segmentationCtrl;
  size_t pduDataFieldLen;

  PduConfig& pduConf;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_HEADERSERIALIZER_H_ */
