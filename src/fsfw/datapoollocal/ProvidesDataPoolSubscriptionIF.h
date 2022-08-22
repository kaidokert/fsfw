#ifndef FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_
#define FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_

#include "fsfw/housekeeping/AcceptsHkPacketsIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "localPoolDefinitions.h"

namespace subdp {

struct ParamsBase {
  ParamsBase(sid_t sid, bool enableReporting, float collectionInterval, bool diagnostics)
      : sid(sid),
        enableReporting(enableReporting),
        collectionInterval(collectionInterval),
        diagnostics(diagnostics) {}

  [[nodiscard]] bool isDiagnostics() const { return diagnostics; }

  sid_t sid;
  bool enableReporting;
  float collectionInterval;
  MessageQueueId_t receiver = MessageQueueIF::NO_QUEUE;

 protected:
  bool diagnostics;
};

struct RegularHkPeriodicParams : public ParamsBase {
  RegularHkPeriodicParams(sid_t sid, bool enableReporting, float collectionInterval)
      : ParamsBase(sid, enableReporting, collectionInterval, false) {}
};

struct DiagnosticsHkPeriodicParams : public ParamsBase {
  DiagnosticsHkPeriodicParams(sid_t sid, bool enableReporting, float collectionInterval)
      : ParamsBase(sid, enableReporting, collectionInterval, true) {}
};

struct RegularHkUpdateParams : public ParamsBase {
  RegularHkUpdateParams(sid_t sid, bool enableReporting)
      : ParamsBase(sid, enableReporting, 0.0, false) {}
};

struct DiagnosticsHkUpdateParams : public ParamsBase {
  DiagnosticsHkUpdateParams(sid_t sid, bool enableReporting)
      : ParamsBase(sid, enableReporting, 0.0, true) {}
};
}  // namespace subdp

class ProvidesDataPoolSubscriptionIF {
 public:
  virtual ~ProvidesDataPoolSubscriptionIF() = default;
  /**
   * @brief   Subscribe for the generation of periodic packets. Used for regular HK packets
   * @details
   * This subscription mechanism will generally be used by the data creator
   * to generate housekeeping packets which are downlinked directly.
   * @return
   */
  virtual ReturnValue_t subscribeForRegularPeriodicPacket(
      subdp::RegularHkPeriodicParams params) = 0;
  /**
   * @brief   Subscribe for the generation of periodic packets. Used for diagnostic packets
   * @details
   * This subscription mechanism will generally be used by the data creator
   * to generate housekeeping packets which are downlinked directly.
   * @return
   */
  virtual ReturnValue_t subscribeForDiagPeriodicPacket(
      subdp::DiagnosticsHkPeriodicParams params) = 0;

  [[deprecated(
      "Please use the new API which takes all arguments as one wrapper "
      "struct")]] virtual ReturnValue_t
  subscribeForPeriodicPacket(sid_t sid, bool enableReporting, float collectionInterval,
                             bool isDiagnostics,
                             object_id_t packetDestination = objects::NO_OBJECT) {
    if (isDiagnostics) {
      subdp::DiagnosticsHkPeriodicParams params(sid, enableReporting, collectionInterval);
      return subscribeForDiagPeriodicPacket(params);
    } else {
      subdp::RegularHkPeriodicParams params(sid, enableReporting, collectionInterval);
      return subscribeForRegularPeriodicPacket(params);
    }
  }

  /**
   * @brief   Subscribe for the  generation of packets if the dataset
   *          is marked as changed.
   * @details
   * This subscription mechanism will generally be used by the data creator.
   * @param sid
   * @param isDiagnostics
   * @param packetDestination
   * @return
   */
  virtual ReturnValue_t subscribeForRegularUpdatePacket(subdp::RegularHkUpdateParams params) = 0;
  virtual ReturnValue_t subscribeForDiagUpdatePacket(subdp::DiagnosticsHkUpdateParams params) = 0;

  //  virtual ReturnValue_t
  //  subscribeForUpdatePacket(sid_t sid, bool reportingEnabled, bool isDiagnostics) {
  //    return subscribeForUpdatePacket(sid, reportingEnabled, isDiagnostics, objects::NO_OBJECT);
  //  }

  [[deprecated(
      "Please use the new API which takes all arguments as one wrapper "
      "struct")]] virtual ReturnValue_t
  subscribeForUpdatePacket(sid_t sid, bool reportingEnabled, bool isDiagnostics,
                           object_id_t packetDestination = objects::NO_OBJECT) {
    if (isDiagnostics) {
      subdp::DiagnosticsHkUpdateParams params(sid, reportingEnabled);
      return subscribeForDiagUpdatePacket(params);
    } else {
      subdp::RegularHkUpdateParams params(sid, reportingEnabled);
      return subscribeForRegularUpdatePacket(params);
    }
  }

  /**
   * @brief   Subscribe for a notification message which will be sent
   *          if a dataset has changed.
   * @details
   * This subscription mechanism will generally be used internally by
   * other software components.
   * @param setId                 Set ID of the set to receive update messages from.
   * @param destinationObject     Object ID of the receiver.
   * @param targetQueueId         Receiver queue ID
   * @param generateSnapshot      If this is set to true, a copy of the current data with a
   *                              timestamp will be generated and sent via message.
   *                              Otherwise, only an notification message is sent.
   * @return
   */
  virtual ReturnValue_t subscribeForSetUpdateMessage(uint32_t setId, object_id_t destinationObject,
                                                     MessageQueueId_t targetQueueId,
                                                     bool generateSnapshot) = 0;
  /**
   * @brief   Subscribe for an notification message which will be sent if a
   *          pool variable has changed.
   * @details
   * This subscription mechanism will generally be used internally by
   * other software components.
   * @param localPoolId           Pool ID of the pool variable
   * @param destinationObject     Object ID of the receiver
   * @param targetQueueId         Receiver queue ID
   * @param generateSnapshot      If this is set to true, a copy of the current data with a
   *                              timestamp will be generated and sent via message. Otherwise,
   *                              only an notification message is sent.
   * @return
   */
  virtual ReturnValue_t subscribeForVariableUpdateMessage(lp_id_t localPoolId,
                                                          object_id_t destinationObject,
                                                          MessageQueueId_t targetQueueId,
                                                          bool generateSnapshot) = 0;
};

#endif /* FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_ */
