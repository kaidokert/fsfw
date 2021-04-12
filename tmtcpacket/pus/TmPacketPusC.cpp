#include "TmPacketPusC.h"
#include "TmPacketBase.h"

#include "../../globalfunctions/CRC.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../timemanager/CCSDSTime.h"

#include <cstring>


TmPacketPusC::TmPacketPusC(uint8_t* setData) : TmPacketBase(setData) {
    tmData = reinterpret_cast<TmPacketPointerPusC*>(setData);
}

TmPacketPusC::~TmPacketPusC() {
    //Nothing to do.
}

uint8_t TmPacketPusC::getService() {
    return tmData->dataField.serviceType;
}

uint8_t TmPacketPusC::getSubService() {
    return tmData->dataField.serviceSubtype;
}

uint8_t* TmPacketPusC::getSourceData() {
    return &tmData->data;
}

uint16_t TmPacketPusC::getSourceDataSize() {
    return getPacketDataLength() - sizeof(tmData->dataField) - CRC_SIZE + 1;
}

void TmPacketPusC::setData(const uint8_t* p_Data) {
    SpacePacketBase::setData(p_Data);
    tmData = (TmPacketPointerPusC*) p_Data;
}


size_t TmPacketPusC::getPacketMinimumSize() const {
    return TM_PACKET_MIN_SIZE;
}

uint16_t TmPacketPusC::getDataFieldSize() {
    return sizeof(PUSTmDataFieldHeaderPusC);
}

uint8_t* TmPacketPusC::getPacketTimeRaw() const{
    return tmData->dataField.time;

}

void TmPacketPusC::initializeTmPacket(uint16_t apid, uint8_t service,
        uint8_t subservice, uint16_t packetSubcounter, uint16_t destinationId,
        uint8_t timeRefField) {
    //Set primary header:
    initSpacePacketHeader(false, true, apid);
    //Set data Field Header:
    //First, set to zero.
    memset(&tmData->dataField, 0, sizeof(tmData->dataField));

    // NOTE: In PUS-C, the PUS Version is 2 and specified for the first 4 bits.
    // The other 4 bits of the first byte are the spacecraft time reference
    // status. To change to PUS-C, set 0b00100000.
    // Set CCSDS_secondary header flag to 0, version number to 001 and ack
    // to 0000
    /* Only account for last 4 bytes */
    timeRefField &= 0b1111;
    tmData->dataField.versionTimeReferenceField = VERSION_NUMBER_BYTE | timeRefField;
    tmData->dataField.serviceType = service;
    tmData->dataField.serviceSubtype = subservice;
    tmData->dataField.subcounterMsb = packetSubcounter << 8 & 0xff;
    tmData->dataField.subcounterLsb = packetSubcounter & 0xff;
    tmData->dataField.destinationIdMsb = destinationId << 8 & 0xff;
    tmData->dataField.destinationIdLsb = destinationId & 0xff;
    //Timestamp packet
    if (TmPacketBase::checkAndSetStamper()) {
        timeStamper->addTimeStamp(tmData->dataField.time,
                sizeof(tmData->dataField.time));
    }
}

void TmPacketPusC::setSourceDataSize(uint16_t size) {
    setPacketDataLength(size + sizeof(PUSTmDataFieldHeaderPusC) + CRC_SIZE - 1);
}

size_t TmPacketPusC::getTimestampSize() const {
    return sizeof(tmData->dataField.time);
}
