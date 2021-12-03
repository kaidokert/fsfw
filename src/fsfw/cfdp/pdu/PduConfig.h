#ifndef FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_

#include "VarLenField.h"

namespace cfdp {

struct EntityId: public VarLenField {
public:
    EntityId(): VarLenField() {}
    EntityId(cfdp::WidthInBytes width, size_t entityId): VarLenField(width, entityId) {}
};

struct TransactionSeqNum: public VarLenField {
public:
    TransactionSeqNum(): VarLenField() {}
    TransactionSeqNum(cfdp::WidthInBytes width, size_t seqNum): VarLenField(width, seqNum) {}
};

}

class PduConfig {
public:
    PduConfig(cfdp::TransmissionModes mode, cfdp::TransactionSeqNum seqNum,
            cfdp::EntityId sourceId, cfdp::EntityId destId, bool crcFlag  = false,
            bool largeFile = false, cfdp::Direction direction = cfdp::Direction::TOWARDS_RECEIVER);
    cfdp::TransmissionModes mode;
    cfdp::TransactionSeqNum seqNum;
    cfdp::EntityId sourceId;
    cfdp::EntityId destId;
    bool crcFlag;
    bool largeFile;
    cfdp::Direction direction;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_ */
