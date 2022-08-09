#ifndef FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PDUCONFIG_H_

#include "fsfw/cfdp/VarLenFields.h"
#include "fsfw/cfdp/definitions.h"

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
