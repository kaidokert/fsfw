#ifndef FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_
#define FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_

#include "../ipc/messageQueueDefinitions.h"
#include "../returnvalues/returnvalue.h"
#include "localPoolDefinitions.h"

class ProvidesDataPoolSubscriptionIF {
 public:
  virtual ~ProvidesDataPoolSubscriptionIF(){};

  /**
   * @brief   Subscribe for the generation of periodic packets.
   * @details
   * This subscription mechanism will generally be used by the data creator
   * to generate housekeeping packets which are downlinked directly.
   * @return
   */
  virtual ReturnValue_t subscribeForPeriodicPacket(sid_t sid, bool enableReporting,
                                                   float collectionInterval, bool isDiagnostics,
                                                   object_id_t packetDestination) = 0;
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
  virtual ReturnValue_t subscribeForUpdatePacket(sid_t sid, bool reportingEnabled,
                                                 bool isDiagnostics,
                                                 object_id_t packetDestination) = 0;
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
  virtual ReturnValue_t subscribeForSetUpdateMessage(const uint32_t setId,
                                                     object_id_t destinationObject,
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
  virtual ReturnValue_t subscribeForVariableUpdateMessage(const lp_id_t localPoolId,
                                                          object_id_t destinationObject,
                                                          MessageQueueId_t targetQueueId,
                                                          bool generateSnapshot) = 0;
};

#endif /* FSFW_DATAPOOLLOCAL_PROVIDESDATAPOOLSUBSCRIPTION_H_ */
