#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE5PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE5PACKETS_H_

#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcservices/VerificationCodes.h"

/**
 * @brief Subservice 1, 2, 3, 4
 * Structure of Event Report.
 * It consists of:
 *      1. Report ID(RID). This is the Event ID in the FSFW
 *      2. Object ID of the reporter (e.g. subsystem)
 *      2. Parameter 1
 *      3. Parameter 2
 *
 * @ingroup spacepackets
 */
class EventReport : public SerializeIF {  //!< [EXPORT] : [SUBSERVICE] 1, 2, 3, 4
 public:
  EventReport(EventId_t reportId_, object_id_t objectId_, uint32_t parameter1_,
              uint32_t parameter2_)
      : reportId(reportId_),
        objectId(objectId_),
        parameter1(parameter1_),
        parameter2(parameter2_) {}

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  SerializeIF::Endianness streamEndianness) const override {
    ReturnValue_t result =
        SerializeAdapter::serialize(&reportId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&objectId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&parameter1, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&parameter2, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return result;
  }

  virtual size_t getSerializedSize() const override {
    uint32_t size = 0;
    size += SerializeAdapter::getSerializedSize(&reportId);
    size += SerializeAdapter::getSerializedSize(&objectId);
    size += SerializeAdapter::getSerializedSize(&parameter1);
    size += SerializeAdapter::getSerializedSize(&parameter2);
    return size;
  }

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    SerializeIF::Endianness streamEndianness) override {
    return returnvalue::FAILED;
  }

 private:
  EventId_t reportId;
  object_id_t objectId;
  uint32_t parameter1;
  uint32_t parameter2;
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE5PACKETS_H_ */
