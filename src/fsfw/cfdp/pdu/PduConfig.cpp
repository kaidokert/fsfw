#include "PduConfig.h"

#include <utility>

PduConfig::PduConfig(cfdp::EntityId sourceId, cfdp::EntityId destId, cfdp::TransmissionMode mode,
                     cfdp::TransactionSeqNum seqNum, bool crcFlag, bool largeFile,
                     cfdp::Direction direction)
    : mode(mode),
      seqNum(std::move(seqNum)),
      sourceId(std::move(sourceId)),
      destId(std::move(destId)),
      crcFlag(crcFlag),
      largeFile(largeFile),
      direction(direction) {}
