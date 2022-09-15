#ifndef FSFW_SRC_FSFW_CFDP_PDU_PDUHEADERIF_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PDUHEADERIF_H_

#include <cstddef>

#include "PduConfig.h"
#include "fsfw/cfdp/definitions.h"

/**
 * @brief   Generic interface to access all fields of a PDU header
 * @details
 * See CCSDS 727.0-B-5 pp.75 for more information about these fields
 */
class PduHeaderIF {
 public:
  virtual ~PduHeaderIF() = default;

  [[nodiscard]] virtual size_t getWholePduSize() const = 0;
  [[nodiscard]] virtual size_t getPduDataFieldLen() const = 0;
  [[nodiscard]] virtual cfdp::PduType getPduType() const = 0;
  [[nodiscard]] virtual cfdp::Direction getDirection() const = 0;
  [[nodiscard]] virtual cfdp::TransmissionMode getTransmissionMode() const = 0;
  [[nodiscard]] virtual bool getCrcFlag() const = 0;
  [[nodiscard]] virtual bool getLargeFileFlag() const = 0;
  [[nodiscard]] virtual cfdp::SegmentationControl getSegmentationControl() const = 0;
  [[nodiscard]] virtual cfdp::WidthInBytes getLenEntityIds() const = 0;
  [[nodiscard]] virtual cfdp::WidthInBytes getLenSeqNum() const = 0;
  [[nodiscard]] virtual cfdp::SegmentMetadataFlag getSegmentMetadataFlag() const = 0;
  [[nodiscard]] virtual bool hasSegmentMetadataFlag() const = 0;
  virtual void getSourceId(cfdp::EntityId& sourceId) const = 0;
  virtual void getDestId(cfdp::EntityId& destId) const = 0;
  virtual void getTransactionSeqNum(cfdp::TransactionSeqNum& seqNum) const = 0;

 private:
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_PDUHEADERIF_H_ */
