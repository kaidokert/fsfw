#include "CCSDSDistributorIF.h"
#include "PUSDistributor.h"

#include "../serviceinterface/ServiceInterface.h"
#include "../tmtcpacket/pus/TcPacketStored.h"
#include "../tmtcservices/PusVerificationReport.h"

#define PUS_DISTRIBUTOR_DEBUGGING     1

PUSDistributor::PUSDistributor(uint16_t setApid, object_id_t setObjectId,
        object_id_t setPacketSource) :
        TcDistributor(setObjectId), checker(setApid), verifyChannel(),
        tcStatus(RETURN_FAILED), packetSource(setPacketSource) {}

PUSDistributor::~PUSDistributor() {}

PUSDistributor::TcMqMapIter PUSDistributor::selectDestination() {
#if FSFW_CPP_OSTREAM_ENABLED == 1 && PUS_DISTRIBUTOR_DEBUGGING == 1
    store_address_t storeId = this->currentMessage.getStorageId());
    sif:: debug << "PUSDistributor::handlePacket received: " << storeId.poolIndex << ", " <<
            storeId.packetIndex << std::endl;
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
            sif::debug << "PUSDistributor::handlePacket: Packet format invalid, code " <<
                    static_cast<int>(tcStatus) << std::endl;
#else
            sif::printDebug("PUSDistributor::handlePacket: Packet format invalid, code %d\n",
                    static_cast<int>(tcStatus));
#endif
#endif
        }
        uint32_t queue_id = currentPacket->getService();
        queueMapIt = this->queueMap.find(queue_id);
    }
    else {
        tcStatus = PACKET_LOST;
    }

    if (queueMapIt == this->queueMap.end()) {
        tcStatus = DESTINATION_NOT_FOUND;
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "PUSDistributor::handlePacket: Destination not found" << std::endl;
#else
        sif::printDebug("PUSDistributor::handlePacket: Destination not found\n");
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


ReturnValue_t PUSDistributor::registerService(AcceptsTelecommandsIF* service) {
    uint16_t serviceId = service->getIdentifier();
#if PUS_DISTRIBUTOR_DEBUGGING == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Service ID: " << static_cast<int>(serviceId) << std::endl;
#else
    sif::printInfo("Service ID: %d\n", static_cast<int>(serviceId));
#endif
#endif
    MessageQueueId_t queue = service->getRequestQueue();
    auto returnPair = queueMap.emplace(serviceId, queue);
    if (not returnPair.second) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PUSDistributor::registerService: Service ID already"
                " exists in map" << std::endl;
#else
        sif::printError("PUSDistributor::registerService: Service ID already exists in map\n");
#endif
#endif
        return SERVICE_ID_ALREADY_EXISTS;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t PUSDistributor::getRequestQueue() {
    return tcQueue->getId();
}

ReturnValue_t PUSDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
    if (queueStatus != RETURN_OK) {
        tcStatus = queueStatus;
    }
    if (tcStatus != RETURN_OK) {
        this->verifyChannel.sendFailureReport(tc_verification::ACCEPTANCE_FAILURE,
                currentPacket, tcStatus);
        // A failed packet is deleted immediately after reporting,
        // otherwise it will block memory.
        currentPacket->deletePacket();
        return RETURN_FAILED;
    } else {
        this->verifyChannel.sendSuccessReport(tc_verification::ACCEPTANCE_SUCCESS,
                currentPacket);
        return RETURN_OK;
    }
}

uint16_t PUSDistributor::getIdentifier() {
    return checker.getApid();
}

ReturnValue_t PUSDistributor::initialize() {
    currentPacket = new TcPacketStored();
    if(currentPacket == nullptr) {
        // Should not happen, memory allocation failed!
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    CCSDSDistributorIF* ccsdsDistributor =
            objectManager->get<CCSDSDistributorIF>(packetSource);
    if (ccsdsDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PUSDistributor::initialize: Packet source invalid" << std::endl;
        sif::error << " Make sure it exists and implements CCSDSDistributorIF!" << std::endl;
#else
        sif::printError("PUSDistributor::initialize: Packet source invalid\n");
        sif::printError("Make sure it exists and implements CCSDSDistributorIF\n");
#endif
        return RETURN_FAILED;
    }
    return ccsdsDistributor->registerApplication(this);
}
