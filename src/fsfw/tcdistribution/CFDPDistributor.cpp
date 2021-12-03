#include "fsfw/tcdistribution/CCSDSDistributorIF.h"
#include "fsfw/tcdistribution/CFDPDistributor.h"

#include "fsfw/tmtcpacket/cfdp/CFDPPacketStored.h"

#include "fsfw/objectmanager/ObjectManager.h"

#ifndef FSFW_CFDP_DISTRIBUTOR_DEBUGGING
#define FSFW_CFDP_DISTRIBUTOR_DEBUGGING     1
#endif

CFDPDistributor::CFDPDistributor(uint16_t setApid, object_id_t setObjectId,
        object_id_t setPacketSource):
        TcDistributor(setObjectId), apid(setApid), checker(setApid),
        tcStatus(RETURN_FAILED), packetSource(setPacketSource) {
}

CFDPDistributor::~CFDPDistributor() {}

CFDPDistributor::TcMqMapIter CFDPDistributor::selectDestination() {
#if FSFW_CFDP_DISTRIBUTOR_DEBUGGING == 1
    store_address_t storeId = this->currentMessage.getStorageId();
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "CFDPDistributor::handlePacket received: " << storeId.poolIndex << ", " <<
            storeId.packetIndex << std::endl;
#else
    sif::printDebug("CFDPDistributor::handlePacket received: %d, %d\n", storeId.poolIndex,
            storeId.packetIndex);
#endif
#endif
    TcMqMapIter queueMapIt = this->queueMap.end();
    if(this->currentPacket == nullptr) {
        return queueMapIt;
    }
    this->currentPacket->setStoreAddress(this->currentMessage.getStorageId());
    if (currentPacket->getWholeData() != nullptr) {
        tcStatus = checker.checkPacket(currentPacket);
        if(tcStatus != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "CFDPDistributor::handlePacket: Packet format invalid, code " <<
                    static_cast<int>(tcStatus) << std::endl;
#else
            sif::printDebug("CFDPDistributor::handlePacket: Packet format invalid, code %d\n",
                    static_cast<int>(tcStatus));
#endif
#endif
        }
        queueMapIt = this->queueMap.find(0);
    }
    else {
        tcStatus = PACKET_LOST;
    }

    if (queueMapIt == this->queueMap.end()) {
        tcStatus = DESTINATION_NOT_FOUND;
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "CFDPDistributor::handlePacket: Destination not found" << std::endl;
#else
        sif::printDebug("CFDPDistributor::handlePacket: Destination not found\n");
#endif /* !FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif
    }

    if (tcStatus != RETURN_OK) {
        return this->queueMap.end();
    }
    else {
        return queueMapIt;
    }

}


ReturnValue_t CFDPDistributor::registerHandler(AcceptsTelecommandsIF* handler) {
    uint16_t handlerId = handler->getIdentifier(); //should be 0, because CFDPHandler does not set a set a service-ID
#if FSFW_CFDP_DISTRIBUTOR_DEBUGGING == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "CFDPDistributor::registerHandler: Handler ID: " << static_cast<int>(handlerId) << std::endl;
#else
    sif::printInfo("CFDPDistributor::registerHandler: Handler ID: %d\n", static_cast<int>(handlerId));
#endif
#endif
    MessageQueueId_t queue = handler->getRequestQueue();
    auto returnPair = queueMap.emplace(handlerId, queue);
    if (not returnPair.second) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CFDPDistributor::registerHandler: Service ID already"
                " exists in map" << std::endl;
#else
        sif::printError("CFDPDistributor::registerHandler: Service ID already exists in map\n");
#endif
#endif
        return SERVICE_ID_ALREADY_EXISTS;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t CFDPDistributor::getRequestQueue() {
    return tcQueue->getId();
}

//ReturnValue_t CFDPDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
//    if (queueStatus != RETURN_OK) {
//        tcStatus = queueStatus;
//    }
//    if (tcStatus != RETURN_OK) {
//        this->verifyChannel.sendFailureReport(tc_verification::ACCEPTANCE_FAILURE,
//                currentPacket, tcStatus);
//        // A failed packet is deleted immediately after reporting,
//        // otherwise it will block memory.
//        currentPacket->deletePacket();
//        return RETURN_FAILED;
//    } else {
//        this->verifyChannel.sendSuccessReport(tc_verification::ACCEPTANCE_SUCCESS,
//                currentPacket);
//        return RETURN_OK;
//    }
//}

uint16_t CFDPDistributor::getIdentifier() {
    return this->apid;
}

ReturnValue_t CFDPDistributor::initialize() {
    currentPacket = new CFDPPacketStored();
    if(currentPacket == nullptr) {
        // Should not happen, memory allocation failed!
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    CCSDSDistributorIF* ccsdsDistributor = ObjectManager::instance()->get<CCSDSDistributorIF>(
            packetSource);
    if (ccsdsDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CFDPDistributor::initialize: Packet source invalid" << std::endl;
        sif::error << " Make sure it exists and implements CCSDSDistributorIF!" << std::endl;
#else
        sif::printError("CFDPDistributor::initialize: Packet source invalid\n");
        sif::printError("Make sure it exists and implements CCSDSDistributorIF\n");
#endif
        return RETURN_FAILED;
    }
    return ccsdsDistributor->registerApplication(this);
}
