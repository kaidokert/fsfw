#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_

#include "../serialize/SerialBufferAdapter.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../datapoollocal/LocalPoolDataSetBase.h"

/**
 * @brief   This helper class will be used to serialize and deserialize
 *          update housekeeping packets into the store.
 */
class HousekeepingPacketUpdate: public SerializeIF {
public:
    /**
     * Update packet constructor for datasets
     * @param timeStamp
     * @param timeStampSize
     * @param hkData
     * @param hkDataSize
     */
    HousekeepingPacketUpdate(uint8_t* timeStamp, size_t timeStampSize,
            LocalPoolDataSetBase* dataSetPtr):
                timeStamp(timeStamp), timeStampSize(timeStampSize),
                updateData(dataSetPtr) {};

    /**
     * Update packet constructor for pool variables.
     * @param timeStamp
     * @param timeStampSize
     * @param dataSetPtr
     */
    HousekeepingPacketUpdate(uint8_t* timeStamp, size_t timeStampSize,
            LocalPoolObjectBase* dataSetPtr):
                timeStamp(timeStamp), timeStampSize(timeStampSize),
                updateData(dataSetPtr) {};

    virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const {
        if(timeStamp != nullptr) {
            /* Endianness will always be MACHINE, so we can simply use memcpy
            here. */
            std::memcpy(*buffer, timeStamp, timeStampSize);
            *size += timeStampSize;
            *buffer += timeStampSize;
        }
        if(updateData == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        return updateData->serialize(buffer, size, maxSize, streamEndianness);
    }

    virtual size_t getSerializedSize() const {
        if(updateData == nullptr) {
            return 0;
        }
        return timeStampSize + updateData->getSerializedSize();
    }

    virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
            SerializeIF::Endianness streamEndianness) override {
        if(timeStamp != nullptr) {
            /* Endianness will always be MACHINE, so we can simply use memcpy
            here. */
            std::memcpy(timeStamp, *buffer, timeStampSize);
            *size -= timeStampSize;
            *buffer += timeStampSize;
        }

        if(updateData == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        return updateData->deSerialize(buffer, size, streamEndianness);
    }

private:
   uint8_t* timeStamp = nullptr;
   size_t timeStampSize = 0;

   SerializeIF* updateData = nullptr;
};


#endif /* FSFW_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_ */
