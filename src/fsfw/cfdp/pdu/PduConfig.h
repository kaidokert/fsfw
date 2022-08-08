#ifndef FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_

#include "VarLenField.h"

namespace cfdp {

struct EntityId : public VarLenField {
 public:
  EntityId() : VarLenField() {}
  template <typename T>
  explicit EntityId(UnsignedByteField<T> byteField) : VarLenField(byteField) {}
  EntityId(cfdp::WidthInBytes width, size_t entityId) : VarLenField(width, entityId) {}
};

struct TransactionSeqNum : public VarLenField {
 public:
  TransactionSeqNum() : VarLenField() {}
  template <typename T>
  explicit TransactionSeqNum(UnsignedByteField<T> byteField) : VarLenField(byteField) {}
  TransactionSeqNum(cfdp::WidthInBytes width, size_t seqNum) : VarLenField(width, seqNum) {}
};

}  // namespace cfdp

class PduConfig {
 public:
  PduConfig(cfdp::EntityId sourceId, cfdp::EntityId destId, cfdp::TransmissionModes mode,
            cfdp::TransactionSeqNum seqNum, bool crcFlag = false, bool largeFile = false,
            cfdp::Direction direction = cfdp::Direction::TOWARDS_RECEIVER);
  cfdp::TransmissionModes mode;
  cfdp::TransactionSeqNum seqNum;
  cfdp::EntityId sourceId;
  cfdp::EntityId destId;
  bool crcFlag;
  bool largeFile;
  cfdp::Direction direction;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_ */
