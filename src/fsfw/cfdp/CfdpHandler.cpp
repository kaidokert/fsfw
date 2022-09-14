#include "fsfw/cfdp/CfdpHandler.h"

#include "fsfw/cfdp/CfdpMessage.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/storagemanager/storeAddress.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

object_id_t CfdpHandler::packetSource = 0;
object_id_t CfdpHandler::packetDestination = 0;

CfdpHandler::CfdpHandler(object_id_t setObjectId, CFDPDistributor* dist)
    : SystemObject(setObjectId) {
  requestQueue = QueueFactory::instance()->createMessageQueue(CFDP_HANDLER_MAX_RECEPTION);
  distributor = dist;
}

CfdpHandler::~CfdpHandler() = default;

ReturnValue_t CfdpHandler::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  this->distributor->registerHandler(this);
  return returnvalue::OK;
}

ReturnValue_t CfdpHandler::handleRequest(store_address_t storeId) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "CFDPHandler::handleRequest" << std::endl;
#else
  sif::printDebug("CFDPHandler::handleRequest\n");
#endif /* !FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif

  // TODO read out packet from store using storeId

  return returnvalue::OK;
}

ReturnValue_t CfdpHandler::performOperation(uint8_t opCode) {
  ReturnValue_t status = returnvalue::OK;
  CommandMessage currentMessage;
  for (status = this->requestQueue->receiveMessage(&currentMessage); status == returnvalue::OK;
       status = this->requestQueue->receiveMessage(&currentMessage)) {
    store_address_t storeId = CfdpMessage::getStoreId(&currentMessage);
    this->handleRequest(storeId);
  }
  return returnvalue::OK;
}

uint32_t CfdpHandler::getIdentifier() const { return 0; }

MessageQueueId_t CfdpHandler::getRequestQueue() const { return this->requestQueue->getId(); }
