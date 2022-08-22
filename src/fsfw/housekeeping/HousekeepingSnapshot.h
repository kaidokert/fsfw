#ifndef FSFW_HOUSEKEEPING_HOUSEKEEPINGSNAPSHOT_H_
#define FSFW_HOUSEKEEPING_HOUSEKEEPINGSNAPSHOT_H_

#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "../datapoollocal/LocalPoolObjectBase.h"
#include "../serialize/SerialBufferAdapter.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../timemanager/CCSDSTime.h"

/**
 * @brief   This helper class will be used to serialize and deserialize update housekeeping packets
 *          into the store.
 */
class HousekeepingSnapshot : public SerializeIF {
 public:
  /**
   * Update packet constructor for datasets.
   * @param cdsShort          If a CSD short timestamp is used, a reference should be
   *                          supplied here
   * @param dataSetPtr        Pointer to the dataset instance to serialize or deserialize the
   *                          data into
   */
  HousekeepingSnapshot(CCSDSTime::CDS_short* cdsShort, LocalPoolDataSetBase* dataSetPtr)
      : timeStamp(reinterpret_cast<uint8_t*>(cdsShort)),
        timeStampSize(sizeof(CCSDSTime::CDS_short)),
        updateData(dataSetPtr){};

  /**
   * Update packet constructor for datasets.
   * @param timeStamp         Pointer to the buffer where the timestamp will be stored.
   * @param timeStampSize     Size of the timestamp
   * @param dataSetPtr        Pointer to the dataset instance to deserialize the data into
   */
  HousekeepingSnapshot(uint8_t* timeStamp, size_t timeStampSize, LocalPoolDataSetBase* dataSetPtr)
      : timeStamp(timeStamp), timeStampSize(timeStampSize), updateData(dataSetPtr){};

  /**
   * Update packet constructor for pool variables.
   * @param timeStamp
   * @param timeStampSize
   * @param dataSetPtr
   */
  HousekeepingSnapshot(CCSDSTime::CDS_short* cdsShort, LocalPoolObjectBase* dataSetPtr)
      : timeStamp(reinterpret_cast<uint8_t*>(cdsShort)),
        timeStampSize(sizeof(CCSDSTime::CDS_short)),
        updateData(dataSetPtr){};

  /**
   * Update packet constructor for pool variables.
   * @param timeStamp
   * @param timeStampSize
   * @param dataSetPtr
   */
  HousekeepingSnapshot(uint8_t* timeStamp, size_t timeStampSize, LocalPoolObjectBase* dataSetPtr)
      : timeStamp(timeStamp), timeStampSize(timeStampSize), updateData(dataSetPtr){};

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const {
    if (timeStamp != nullptr) {
      /* Endianness will always be MACHINE, so we can simply use memcpy
      here. */
      std::memcpy(*buffer, timeStamp, timeStampSize);
      *size += timeStampSize;
      *buffer += timeStampSize;
    }
    if (updateData == nullptr) {
      return returnvalue::FAILED;
    }

    return updateData->serialize(buffer, size, maxSize, streamEndianness);
  }

  virtual size_t getSerializedSize() const {
    if (updateData == nullptr) {
      return 0;
    }
    return timeStampSize + updateData->getSerializedSize();
  }

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    SerializeIF::Endianness streamEndianness) override {
    if (*size < timeStampSize) {
      return SerializeIF::STREAM_TOO_SHORT;
    }

    if (timeStamp != nullptr) {
      /* Endianness will always be MACHINE, so we can simply use memcpy
      here. */
      std::memcpy(timeStamp, *buffer, timeStampSize);
      *size -= timeStampSize;
      *buffer += timeStampSize;
    }

    if (updateData == nullptr) {
      return returnvalue::FAILED;
    }
    if (*size < updateData->getSerializedSize()) {
      return SerializeIF::STREAM_TOO_SHORT;
    }

    return updateData->deSerialize(buffer, size, streamEndianness);
  }

 private:
  uint8_t* timeStamp = nullptr;
  size_t timeStampSize = 0;

  SerializeIF* updateData = nullptr;
};

#endif /* FSFW_HOUSEKEEPING_HOUSEKEEPINGSNAPSHOT_H_ */
