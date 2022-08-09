#ifndef FSFW_CFDP_FAULTHANDLERBASE_H
#define FSFW_CFDP_FAULTHANDLERBASE_H

#include <etl/flat_map.h>

#include "definitions.h"

class CfdpFaultHandlerBase {
 public:
  virtual ~CfdpFaultHandlerBase();
  CfdpFaultHandlerBase();

 private:
  etl::flat_map<cfdp::ConditionCode, cfdp::FaultHandlerCodes, 9> handleMap = {
      etl::pair{cfdp::ConditionCode::POSITIVE_ACK_LIMIT_REACHED,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::KEEP_ALIVE_LIMIT_REACHED,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::INVALID_TRANSMISSION_MODE,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::FILE_CHECKSUM_FAILURE, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::FILE_SIZE_ERROR, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::NAK_LIMIT_REACHED, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::INACTIVITY_DETECTED, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCode::UNSUPPORTED_CHECKSUM_TYPE,
                cfdp::FaultHandlerCodes::IGNORE_ERROR}};
};

#endif  // FSFW_CFDP_FAULTHANDLERBASE_H
