#ifndef FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_
#define FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_

#include <framework/datapoollocal/LocalPoolDataSetBase.h>
#include <framework/housekeeping/HousekeepingMessage.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/storagemanager/StorageManagerIF.h>

/**
 * @brief   This class will be used to serialize general housekeeping packets
 *          which are destined to be downlinked into the store.
 * @details
 * The housekeeping packets are stored into the IPC store and forwarded
 * to the designated housekeeping handler.
 */
class HousekeepingPacketDownlink: public SerialLinkedListAdapter<SerializeIF> {
public:
    HousekeepingPacketDownlink(sid_t sid, float collectionInterval, uint8_t
            numberOfParameters, LocalPoolDataSetBase* dataSetPtr):
            sourceId(sid.objectId), setId(sid.ownerSetId),
            collectionInterval(collectionInterval),
            numberOfParameters(numberOfParameters), hkData(dataSetPtr) {
        setLinks();
    }

    /**
     * Helper functions which can be used to move HK data from the IPC store
     * to the telemetry store.
     * @param formerStore
     * @param storeId
     * @param newStore
     * @param newStoreId [out]
     * @return
     */
    virtual ReturnValue_t moveToOtherStore(StorageManagerIF* formerStore,
            store_address_t storeId, StorageManagerIF* newStore,
            store_address_t* newStoreId) {
        const uint8_t* dataPtr = nullptr;
        size_t hkDataSize = 0;
        ReturnValue_t result = formerStore->getData(storeId, &dataPtr,
                &hkDataSize);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        return newStore->addData(newStoreId, dataPtr, hkDataSize);
    }

private:
    void setLinks() {
        setStart(&sourceId);
        sourceId.setNext(&setId);
        setId.setNext(&collectionInterval);
        collectionInterval.setNext(&numberOfParameters);
        numberOfParameters.setNext(&hkData);
    }

    SerializeElement<object_id_t> sourceId;
    SerializeElement<uint32_t> setId;
    SerializeElement<float> collectionInterval;
    SerializeElement<uint8_t> numberOfParameters;
    LinkedElement<SerializeIF> hkData;
};

#endif /* FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETDOWNLINK_H_ */



//    virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
//                size_t maxSize, Endianness streamEndianness) const override {
//        ReturnValue_t result = SerialLinkedListAdapter::serialize(buffer, size,
//                maxSize, streamEndianness);
//        if(result != HasReturnvaluesIF::RETURN_OK) {
//            return result;
//        }
//        return dataSetPtr->serialize(buffer, size, maxSize, streamEndianness);
//   }
//
//   virtual size_t getSerializedSize() const override {
//
//   }
