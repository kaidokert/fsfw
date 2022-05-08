#include "fsfw/cfdp/CFDPHandler.h"

#include "fsfw/cfdp/CFDPMessage.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/storagemanager/storeAddress.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

object_id_t CFDPHandler::packetSource = 0;
object_id_t CFDPHandler::packetDestination = 0;

CFDPHandler::CFDPHandler(object_id_t setObjectId, CFDPDistributor* dist)
    : SystemObject(setObjectId) {
  requestQueue = QueueFactory::instance()->createMessageQueue(CFDP_HANDLER_MAX_RECEPTION);
  distributor = dist;
}

CFDPHandler::~CFDPHandler() {}

ReturnValue_t CFDPHandler::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != RETURN_OK) {
    return result;
  }
  this->distributor->registerHandler(this);
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CFDPHandler::handleRequest(store_address_t storeId) {
  FSFW_LOGDT("{}", "CFDPHandler::handleRequest\n");

  // TODO read out packet from store using storeId

  return RETURN_OK;
}

ReturnValue_t CFDPHandler::performOperation(uint8_t opCode) {
  ReturnValue_t status = RETURN_OK;
  CommandMessage currentMessage;
  for (status = this->requestQueue->receiveMessage(&currentMessage); status == RETURN_OK;
       status = this->requestQueue->receiveMessage(&currentMessage)) {
    store_address_t storeId = CFDPMessage::getStoreId(&currentMessage);
    this->handleRequest(storeId);
  }
  return RETURN_OK;
}

uint16_t CFDPHandler::getIdentifier() { return 0; }

MessageQueueId_t CFDPHandler::getRequestQueue() { return this->requestQueue->getId(); }
