#ifndef FSFW_TMSTORAGE_TMSTOREPACKETS_H_
#define FSFW_TMSTORAGE_TMSTOREPACKETS_H_

#include "fsfw/globalfunctions/timevalOperations.h"
#include "fsfw/serialize/SerialBufferAdapter.h"
#include "fsfw/serialize/SerialFixedArrayListAdapter.h"
#include "fsfw/serialize/SerialLinkedListAdapter.h"
#include "fsfw/serialize/SerializeElement.h"
#include "fsfw/timemanager/CCSDSTime.h"
#include "fsfw/timemanager/TimeWriterIF.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmMinimal.h"
#include "tmStorageConf.h"

class ServiceSubservice : public SerialLinkedListAdapter<SerializeIF> {
 public:
  SerializeElement<uint8_t> service;
  SerialFixedArrayListAdapter<uint8_t, 16> subservices;
  LinkedElement<SerializeIF> linkedSubservices;
  ServiceSubservice()
      : SerialLinkedListAdapter<SerializeIF>(&service), linkedSubservices(&subservices) {
    service.setNext(&linkedSubservices);
  }
};

class ApidSsc : public SerializeIF {
 public:
  ApidSsc() : apid(SpacePacketBase::LIMIT_APID), ssc(0) {}
  ApidSsc(uint16_t apid, uint16_t ssc) : apid(apid), ssc(ssc) {}
  uint16_t apid;
  uint16_t ssc;
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const {
    ReturnValue_t result =
        SerializeAdapter::serialize(&apid, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return SerializeAdapter::serialize(&ssc, buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const { return sizeof(apid) + sizeof(ssc); }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
    ReturnValue_t result = SerializeAdapter::deSerialize(&apid, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return SerializeAdapter::deSerialize(&ssc, buffer, size, streamEndianness);
  }
};

class ChangeSelectionDefinition : public SerialLinkedListAdapter<SerializeIF> {
 public:
  SerializeElement<uint16_t> apid;
  SerialFixedArrayListAdapter<ServiceSubservice, 16> serviceList;
  LinkedElement<SerializeIF> linkedServiceList;
  ChangeSelectionDefinition()
      : SerialLinkedListAdapter<SerializeIF>(&apid), linkedServiceList(&serviceList) {
    apid.setNext(&linkedServiceList);
  }
};

class TmPacketInformation : public SerializeIF {
 public:
  TmPacketInformation(TmPacketMinimal* packet) { setContent(packet); }
  TmPacketInformation()
      : apid(SpacePacketBase::LIMIT_APID),
        sourceSequenceCount(0),
        serviceType(0),
        serviceSubtype(0),
        subCounter(0) {}
  void reset() {
    apid = SpacePacketBase::LIMIT_APID;
    sourceSequenceCount = 0;
    serviceType = 0;
    serviceSubtype = 0;
    subCounter = 0;
    memset(rawTimestamp, 0, sizeof(rawTimestamp));
  }
  void setContent(TmPacketMinimal* packet) {
    apid = packet->getAPID();
    sourceSequenceCount = packet->getPacketSequenceCount();
    serviceType = packet->getService();
    serviceSubtype = packet->getSubService();
    subCounter = packet->getPacketSubcounter();
    memset(rawTimestamp, 0, sizeof(rawTimestamp));
    const uint8_t* pField = NULL;
    uint32_t size = 0;
    ReturnValue_t result = packet->getPacketTimeRaw(&pField, &size);
    if (result != returnvalue::OK) {
      return;
    }
    if (*pField == CCSDSTime::P_FIELD_CDS_SHORT && size <= TimeStamperIF::MISSION_TIMESTAMP_SIZE) {
      // Shortcut to avoid converting CDS back and forth.
      memcpy(rawTimestamp, pField, size);
      return;
    }
    timeval time = {0, 0};
    result = packet->getPacketTime(&time);
    if (result != returnvalue::OK) {
      return;
    }

    CCSDSTime::CDS_short cdsFormat;
    result = CCSDSTime::convertToCcsds(&cdsFormat, &time);
    if (result != returnvalue::OK) {
      return;
    }
    memcpy(rawTimestamp, &cdsFormat, sizeof(cdsFormat));
  }
  void setContent(TmPacketInformation* content) {
    apid = content->apid;
    sourceSequenceCount = content->sourceSequenceCount;
    serviceType = content->serviceType;
    serviceSubtype = content->serviceSubtype;
    subCounter = content->subCounter;
    memcpy(rawTimestamp, content->rawTimestamp, sizeof(rawTimestamp));
  }
  bool isValid() const { return (apid < SpacePacketBase::LIMIT_APID) ? true : false; }
  static void reset(TmPacketInformation* packet) { packet->reset(); }

  static bool isOlderThan(const TmPacketInformation* packet, const timeval* cmpTime) {
    if (packet->isValid()) {
      timeval packetTime = {0, 0};
      size_t foundlen = 0;
      CCSDSTime::convertFromCcsds(&packetTime, &packet->rawTimestamp[0], &foundlen,
                                  sizeof(rawTimestamp));
      if (packetTime <= *cmpTime) {
        return true;
      }
    }
    return false;
  }

  static bool isNewerThan(const TmPacketInformation* packet, const timeval* cmpTime) {
    if (packet->isValid()) {
      timeval packetTime = {0, 0};
      size_t foundlen = 0;
      CCSDSTime::convertFromCcsds(&packetTime, &packet->rawTimestamp[0], &foundlen,
                                  sizeof(rawTimestamp));
      if (packetTime >= *cmpTime) {
        return true;
      }
    }
    return false;
  }

  static bool isSmallerSSC(const TmPacketInformation* packet, const ApidSsc* compareSSC) {
    if (packet->isValid()) {
      if (packet->apid == compareSSC->apid) {
        if (packet->sourceSequenceCount <= compareSSC->ssc) {
          return true;
        }
      }
    }
    return false;
  }

  static bool isLargerSSC(const TmPacketInformation* packet, const ApidSsc* compareSSC) {
    if (packet->isValid()) {
      if (packet->apid == compareSSC->apid) {
        if (packet->sourceSequenceCount >= compareSSC->ssc) {
          return true;
        }
      }
    }
    return false;
  }

  uint16_t getApid() const { return apid; }

  uint16_t getSsc() const { return sourceSequenceCount; }

  uint8_t getServiceType() const { return serviceType; }

  uint8_t getServiceSubtype() const { return serviceSubtype; }

  uint8_t getSubCounter() const { return subCounter; }

  timeval getTime() const {
    timeval packetTime = {0, 0};
    size_t foundlen = 0;
    CCSDSTime::convertFromCcsds(&packetTime, &this->rawTimestamp[0], &foundlen,
                                sizeof(rawTimestamp));
    return packetTime;
  }

  bool operator==(const TmPacketInformation& other) {
    // TODO Does not compare Raw Timestamp
    return ((apid == other.getApid()) && (sourceSequenceCount == other.getSsc()) &&
            (serviceType == other.getServiceType()) &&
            (serviceSubtype = other.getServiceSubtype()));
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const {
    ReturnValue_t result =
        SerializeAdapter::serialize(&apid, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result =
        SerializeAdapter::serialize(&sourceSequenceCount, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&serviceType, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&serviceSubtype, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&subCounter, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    SerialBufferAdapter<uint8_t> adapter(rawTimestamp, sizeof(rawTimestamp));
    return adapter.serialize(buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const {
    uint32_t size = 0;
    size += SerializeAdapter::getSerializedSize(&apid);
    size += SerializeAdapter::getSerializedSize(&sourceSequenceCount);
    size += SerializeAdapter::getSerializedSize(&serviceType);
    size += SerializeAdapter::getSerializedSize(&serviceSubtype);
    size += SerializeAdapter::getSerializedSize(&subCounter);
    SerialBufferAdapter<uint8_t> adapter(rawTimestamp, sizeof(rawTimestamp));
    size += adapter.getSerializedSize();
    return size;
  };

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
    ReturnValue_t result = SerializeAdapter::deSerialize(&apid, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&sourceSequenceCount, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&serviceType, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&serviceSubtype, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&subCounter, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    SerialBufferAdapter<uint8_t> adapter(rawTimestamp, sizeof(rawTimestamp));
    return adapter.deSerialize(buffer, size, streamEndianness);
  }

 private:
  uint16_t apid;
  uint16_t sourceSequenceCount;
  uint8_t serviceType;
  uint8_t serviceSubtype;
  uint8_t subCounter;
  uint8_t rawTimestamp[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
};
#endif /* FSFW_TMSTORAGE_TMSTOREPACKETS_H_ */
