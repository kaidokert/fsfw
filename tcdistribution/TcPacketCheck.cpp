#include "TcPacketCheck.h"

#include "../globalfunctions/CRC.h"
#include "../tmtcpacket/pus/tc/TcPacketBase.h"
#include "../tmtcpacket/pus/tc/TcPacketStoredBase.h"
#include "../serviceinterface/ServiceInterface.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tmtcservices/VerificationCodes.h"

TcPacketCheck::TcPacketCheck(uint16_t setApid): apid(setApid) {
}

ReturnValue_t TcPacketCheck::checkPacket(TcPacketStoredBase* currentPacket) {
    TcPacketBase* tcPacketBase = currentPacket->getPacketBase();
    if(tcPacketBase == nullptr) {
        return RETURN_FAILED;
    }
    uint16_t calculated_crc = CRC::crc16ccitt(tcPacketBase->getWholeData(),
            tcPacketBase->getFullSize());
    if (calculated_crc != 0) {
        return INCORRECT_CHECKSUM;
    }
    bool condition = (not tcPacketBase->hasSecondaryHeader()) or
            (tcPacketBase->getPacketVersionNumber() != CCSDS_VERSION_NUMBER) or
            (not tcPacketBase->isTelecommand());
    if (condition) {
        return INCORRECT_PRIMARY_HEADER;
    }
    if (tcPacketBase->getAPID() != this->apid)
        return ILLEGAL_APID;

    if (not currentPacket->isSizeCorrect()) {
        return INCOMPLETE_PACKET;
    }
    condition = (tcPacketBase->getSecondaryHeaderFlag() != CCSDS_SECONDARY_HEADER_FLAG) ||
            (tcPacketBase->getPusVersionNumber() != PUS_VERSION_NUMBER);
    if (condition) {
        return INCORRECT_SECONDARY_HEADER;
    }
    return RETURN_OK;
}

uint16_t TcPacketCheck::getApid() const {
    return apid;
}
