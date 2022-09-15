#ifndef FSFW_CFDP_FAULTHANDLERBASE_H
#define FSFW_CFDP_FAULTHANDLERBASE_H

#include <etl/flat_map.h>

#include "fsfw/cfdp/VarLenFields.h"
#include "fsfw/cfdp/definitions.h"

namespace cfdp {

/**
 * @brief Provides a way to implement the fault handling procedures as specified
 * in chapter 4.8 of the CFDP standard.
 *
 * @details
 * It is passed into the CFDP handlers as part of the local entity configuration and provides
 * a way to specify custom user error handlers.
 *
 * It does so by mapping each applicable CFDP condition code to a fault handler which
 * is denoted by the four @cfdp::FaultHandlerCodes. This code is used to dispatch
 * to a user-provided callback function:
 *
 * 1. @FaultHandlerCodes::IGNORE_ERROR -> @ignore_cb
 * 2. @FaultHandlerCodes::NOTICE_OF_CANCELLATION` -> @notice_of_cancellation_cb
 * 3. @FaultHandlerCodes::NOTICE_OF_SUSPENSION` -> @notice_of_suspension_cb
 * 4. @FaultHandlerCodes::ABANDON_TRANSACTION` -> @abandon_transaction_cb
 *
 * For each error reported by @reportError, the appropriate fault handler callback
 * will be called. The user provides the callbacks by providing a custom class which implements
 * these base class and all abstract fault handler callbacks.
 */
class FaultHandlerBase {
 public:
  virtual ~FaultHandlerBase();
  FaultHandlerBase();

  /**
   * Get the fault handler code for the given condition code
   * @param code
   * @param handler [out] Will be set to the approrpiate handler for the condition code if
   *    it is valid
   * @return
   *  - true if the condition code is valid
   *  - false otherwise
   */
  bool getFaultHandler(cfdp::ConditionCodes code, cfdp::FaultHandlerCodes& handler) const;

  bool setFaultHandler(cfdp::ConditionCodes code, cfdp::FaultHandlerCodes handler);

  bool reportFault(cfdp::TransactionId& id, cfdp::ConditionCodes code);

  virtual void noticeOfSuspensionCb(cfdp::TransactionId& id, cfdp::ConditionCodes code) = 0;
  virtual void noticeOfCancellationCb(cfdp::TransactionId& id, cfdp::ConditionCodes code) = 0;
  virtual void abandonCb(cfdp::TransactionId& id, cfdp::ConditionCodes code) = 0;
  virtual void ignoreCb(cfdp::TransactionId& id, cfdp::ConditionCodes code) = 0;

 private:
  etl::flat_map<cfdp::ConditionCodes, cfdp::FaultHandlerCodes, 10> faultHandlerMap = {
      etl::pair{cfdp::ConditionCodes::POSITIVE_ACK_LIMIT_REACHED,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::KEEP_ALIVE_LIMIT_REACHED,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::INVALID_TRANSMISSION_MODE,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::FILE_CHECKSUM_FAILURE, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::FILE_SIZE_ERROR, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::NAK_LIMIT_REACHED, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::INACTIVITY_DETECTED, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::UNSUPPORTED_CHECKSUM_TYPE,
                cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::FILESTORE_REJECTION, cfdp::FaultHandlerCodes::IGNORE_ERROR},
      etl::pair{cfdp::ConditionCodes::CHECK_LIMIT_REACHED, cfdp::FaultHandlerCodes::IGNORE_ERROR}};
};

}  // namespace cfdp

#endif  // FSFW_CFDP_FAULTHANDLERBASE_H
