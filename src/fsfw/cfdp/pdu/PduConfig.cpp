#include "PduConfig.h"

PduConfig::PduConfig(cfdp::TransmissionModes mode, cfdp::TransactionSeqNum seqNum,
                     cfdp::EntityId sourceId, cfdp::EntityId destId, bool crcFlag, bool largeFile,
                     cfdp::Direction direction)
    : mode(mode),
      seqNum(seqNum),
      sourceId(sourceId),
      destId(destId),
      crcFlag(crcFlag),
      largeFile(largeFile),
      direction(direction) {}
