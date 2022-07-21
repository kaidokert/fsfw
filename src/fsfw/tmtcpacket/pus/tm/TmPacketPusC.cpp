#include "TmPacketPusC.h"

#include <cstring>

#include "../defs.h"
#include "TmPacketBase.h"
#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/timemanager/CCSDSTime.h"

TmPacketPusC::TmPacketPusC(uint8_t* setData) : TmPacketBase(setData) {
  tmData = reinterpret_cast<TmPacketPointerPusC*>(setData);
}

TmPacketPusC::~TmPacketPusC() = default;

uint8_t TmPacketPusC::getService() { return tmData->dataField.serviceType; }

uint8_t TmPacketPusC::getSubService() { return tmData->dataField.serviceSubtype; }

uint8_t* TmPacketPusC::getSourceData() { return &tmData->data; }

uint16_t TmPacketPusC::getSourceDataSize() {
  return SpacePacketReader::getPacketDataLen() - sizeof(tmData->dataField) - CRC_SIZE + 1;
}

ReturnValue_t TmPacketPusC::setData(uint8_t* p_Data, size_t maxSize, void* args) {
  ReturnValue_t result = SpacePacketReader::setData(p_Data, maxSize, args);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (maxSize < sizeof(TmPacketPointerPusC)) {
    return HasReturnvaluesIF::RETURN_OK;
  }
  tmData = reinterpret_cast<TmPacketPointerPusC*>(const_cast<uint8_t*>(p_Data));
  return HasReturnvaluesIF::RETURN_OK;
}

size_t TmPacketPusC::getPacketMinimumSize() const { return TM_PACKET_MIN_SIZE; }

uint16_t TmPacketPusC::getDataFieldSize() { return sizeof(PUSTmDataFieldHeaderPusC); }

uint8_t* TmPacketPusC::getPacketTimeRaw() const { return tmData->dataField.time; }

ReturnValue_t TmPacketPusC::initializeTmPacket(uint16_t apid, uint8_t service, uint8_t subservice,
                                               uint16_t packetSubcounter, uint16_t destinationId,
                                               uint8_t timeRefField) {
  // Set primary header:
  ReturnValue_t result = initSpacePacketHeader(false, true, apid);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  // Set data Field Header:
  // First, set to zero.
  memset(&tmData->dataField, 0, sizeof(tmData->dataField));

  /* Only account for last 4 bytes for time reference field */
  timeRefField &= 0b1111;
  tmData->dataField.versionTimeReferenceField = (ecss::PusVersion::PUS_C << 4) | timeRefField;
  tmData->dataField.serviceType = service;
  tmData->dataField.serviceSubtype = subservice;
  tmData->dataField.subcounterMsb = (packetSubcounter << 8) & 0xff;
  tmData->dataField.subcounterLsb = packetSubcounter & 0xff;
  tmData->dataField.destinationIdMsb = (destinationId << 8) & 0xff;
  tmData->dataField.destinationIdLsb = destinationId & 0xff;
  // Timestamp packet
  if (TmPacketBase::checkAndSetStamper()) {
    timeStamper->addTimeStamp(tmData->dataField.time, sizeof(tmData->dataField.time));
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void TmPacketPusC::setSourceDataSize(uint16_t size) {
  setPacketDataLength(size + sizeof(PUSTmDataFieldHeaderPusC) + CRC_SIZE - 1);
}

size_t TmPacketPusC::getTimestampSize() const { return sizeof(tmData->dataField.time); }
