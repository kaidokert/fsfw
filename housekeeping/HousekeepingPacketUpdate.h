#ifndef FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_
#define FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_

#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/datapoollocal/LocalPoolDataSetBase.h>

/**
 * @brief   This helper class will be used to serialize and deserialize
 *          internal housekeeping packets into the store.
 */
class HousekeepingPacketUpdate: public SerializeIF {
public:
    /**
     * @param timeStamp
     * @param timeStampSize
     * @param hkData
     * @param hkDataSize
     */
    HousekeepingPacketUpdate(uint8_t* timeStamp, size_t timeStampSize,
            LocalPoolDataSetBase* dataSetPtr):
            timeStamp(timeStamp), timeStampSize(timeStampSize),
            dataSetPtr(dataSetPtr) {};

    virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const {
        if(timeStamp != nullptr) {
            // Endianness will always be MACHINE, so we can simply use memcpy
            // here.
            std::memcpy(*buffer, timeStamp, timeStampSize);
            *size += timeStampSize;
            *buffer += timeStampSize;
        }
        if(dataSetPtr == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        return dataSetPtr->serialize(buffer, size, maxSize, streamEndianness);
    }

    virtual size_t getSerializedSize() const {
        if(dataSetPtr == nullptr) {
            return 0;
        }
        return timeStampSize + dataSetPtr->getSerializedSize();
    }

    virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
            SerializeIF::Endianness streamEndianness) override {
        if(timeStamp != nullptr) {
            // Endianness will always be MACHINE, so we can simply use memcpy
            // here.
            std::memcpy(timeStamp, *buffer, timeStampSize);
            *size += timeStampSize;
            *buffer += timeStampSize;
        }

        if(dataSetPtr == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        return dataSetPtr->deSerialize(buffer, size, streamEndianness);
    }

private:
   uint8_t* timeStamp;
   size_t timeStampSize;

   LocalPoolDataSetBase* dataSetPtr = nullptr;
};



#endif /* FRAMEWORK_HOUSEKEEPING_HOUSEKEEPINGPACKETUPDATE_H_ */
