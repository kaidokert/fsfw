#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_

#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../storagemanager/StorageManagerIF.h"

/**
 * @brief   This class will be used to serialize general housekeeping packets
 *          which are destined to be downlinked into the store.
 * @details
 * The housekeeping packets are stored into the IPC store and forwarded
 * to the designated housekeeping handler. The packet will consist of the following fields
 *  - SID (8 byte): Structure ID, with the first 4 bytes being the object ID and the last four
 *    bytes being the set ID
 *  - Housekeeping Data: The rest of the packet will be the serialized housekeeping data. A validity
 *    buffer might be appended at the end, depending on the set configuration.
 */
class HousekeepingPacketDownlink : public SerialLinkedListAdapter<SerializeIF> {
 public:
  HousekeepingPacketDownlink(sid_t sid, LocalPoolDataSetBase* dataSetPtr)
      : sourceId(sid.objectId), setId(sid.ownerSetId), hkData(dataSetPtr) {
    setLinks();
  }

 private:
  void setLinks() {
    setStart(&sourceId);
    sourceId.setNext(&setId);
    setId.setNext(&hkData);
  }

  SerializeElement<object_id_t> sourceId;
  SerializeElement<uint32_t> setId;
  LinkedElement<SerializeIF> hkData;
};

#endif /* FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_ */
