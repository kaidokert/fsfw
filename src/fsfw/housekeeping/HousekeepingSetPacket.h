#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGSETPACKET_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGSETPACKET_H_

#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "../housekeeping/HousekeepingMessage.h"
#include "../serialize/SerialLinkedListAdapter.h"

class HousekeepingSetPacket : public SerialLinkedListAdapter<SerializeIF> {
 public:
  HousekeepingSetPacket(sid_t sid, bool reportingEnabled, bool valid, float collectionInterval,
                        LocalPoolDataSetBase* dataSetPtr)
      : objectId(sid.objectId),
        setId(sid.ownerSetId),
        reportingEnabled(reportingEnabled),
        valid(valid),
        collectionIntervalSeconds(collectionInterval),
        dataSet(dataSetPtr) {
    setLinks();
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    ReturnValue_t result =
        SerialLinkedListAdapter::serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return dataSet->serializeLocalPoolIds(buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const override {
    size_t linkedSize = SerialLinkedListAdapter::getSerializedSize();
    linkedSize += dataSet->getLocalPoolIdsSerializedSize();
    return linkedSize;
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    return returnvalue::OK;
  }

 private:
  void setLinks() {
    setStart(&objectId);
    objectId.setNext(&setId);
    setId.setNext(&reportingEnabled);
    reportingEnabled.setNext(&valid);
    valid.setNext(&collectionIntervalSeconds);
    collectionIntervalSeconds.setEnd();
  }

  SerializeElement<object_id_t> objectId;
  SerializeElement<uint32_t> setId;
  SerializeElement<bool> reportingEnabled;
  SerializeElement<bool> valid;
  SerializeElement<float> collectionIntervalSeconds;
  LocalPoolDataSetBase* dataSet;
};

#endif /* FSFW_HOUSEKEEPING_HOUSEKEEPINGSETPACKET_H_ */
