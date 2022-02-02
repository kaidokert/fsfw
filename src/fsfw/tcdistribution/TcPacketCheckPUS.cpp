#include "fsfw/tcdistribution/TcPacketCheckPUS.h"

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/tmtcpacket/pus/tc/TcPacketStoredPus.h"
#include "fsfw/tmtcpacket/pus/tc/TcPacketPusBase.h"
#include "fsfw/tmtcpacket/pus/tc/TcPacketStoredBase.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tmtcservices/VerificationCodes.h"

TcPacketCheckPUS::TcPacketCheckPUS(uint16_t setApid): apid(setApid) {
}

ReturnValue_t TcPacketCheckPUS::checkPacket(SpacePacketBase* currentPacket) {
    TcPacketStoredBase* storedPacket = dynamic_cast<TcPacketStoredBase*>(currentPacket);
    TcPacketPusBase* tcPacketBase = dynamic_cast<TcPacketPusBase*>(currentPacket);
    if(tcPacketBase == nullptr or storedPacket == nullptr) {
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

    if (not storedPacket->isSizeCorrect()) {
        return INCOMPLETE_PACKET;
    }

    condition = (tcPacketBase->getSecondaryHeaderFlag() != CCSDS_SECONDARY_HEADER_FLAG) ||
            (tcPacketBase->getPusVersionNumber() != PUS_VERSION_NUMBER);
    if (condition) {
        return INCORRECT_SECONDARY_HEADER;
    }
    return RETURN_OK;
}

uint16_t TcPacketCheckPUS::getApid() const {
    return apid;
}
