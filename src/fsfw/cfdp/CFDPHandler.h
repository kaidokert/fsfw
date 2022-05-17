#ifndef FSFW_CFDP_CFDPHANDLER_H_
#define FSFW_CFDP_CFDPHANDLER_H_

#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tcdistribution/CFDPDistributor.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

namespace Factory {
void setStaticFrameworkObjectIds();
}

class CFDPHandler : public ExecutableObjectIF,
                    public AcceptsTelecommandsIF,
                    public SystemObject,
                    public HasReturnvaluesIF {
  friend void(Factory::setStaticFrameworkObjectIds)();

 public:
  CFDPHandler(object_id_t setObjectId, CFDPDistributor* distributor);
  /**
   * The destructor is empty.
   */
  virtual ~CFDPHandler();

  virtual ReturnValue_t handleRequest(store_address_t storeId);

  virtual ReturnValue_t initialize() override;
  virtual uint16_t getIdentifier() override;
  MessageQueueId_t getRequestQueue() override;
  ReturnValue_t performOperation(uint8_t opCode) override;

 protected:
  /**
   * This is a complete instance of the telecommand reception queue
   * of the class. It is initialized on construction of the class.
   */
  MessageQueueIF* requestQueue = nullptr;

  CFDPDistributor* distributor = nullptr;

  /**
   * The current CFDP packet to be processed.
   * It is deleted after handleRequest was executed.
   */
  CFDPPacketStored currentPacket;

  static object_id_t packetSource;

  static object_id_t packetDestination;

 private:
  /**
   * This constant sets the maximum number of packets accepted per call.
   * Remember that one packet must be completely handled in one
   * #handleRequest call.
   */
  static const uint8_t CFDP_HANDLER_MAX_RECEPTION = 100;
};

#endif /* FSFW_CFDP_CFDPHANDLER_H_ */
