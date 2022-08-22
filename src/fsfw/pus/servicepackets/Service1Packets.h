#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE1PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE1PACKETS_H_

/**
 * @defgroup spacepackets PUS Packet Definitions
 *  This group contains all implemented TM or TM packages that are sent to
 *  or sent by the OBC.They are exported later to display
 *  packet structures in Mission Information Base (MIB).
 */

#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcservices/VerificationCodes.h"

/**
 * @brief 	FailureReport class to serialize a failure report
 * @brief 	Subservice 2, 4, 6, 8
 * @ingroup spacepackets
 */
class FailureReport : public SerializeIF {  //!< [EXPORT] : [SUBSERVICE] 2, 4, 6, 8
 public:
  FailureReport(uint8_t failureSubtype_, uint16_t packetId_, uint16_t packetSequenceControl_,
                uint8_t stepNumber_, ReturnValue_t errorCode_, uint32_t errorParameter1_,
                uint32_t errorParameter2_)
      : packetId(packetId_),
        packetSequenceControl(packetSequenceControl_),
        stepNumber(stepNumber_),
        errorCode(errorCode_),
        errorParameter1(errorParameter1_),
        errorParameter2(errorParameter2_),
        failureSubtype(failureSubtype_) {}

  /**
   * This function is called by the FSFW when calling the tm packet send
   * function and supplying the SerializeIF* as parameter
   * @param buffer Object content is serialized into the buffer
   * @param size
   * @param max_size
   * @param bigEndian
   * @return
   */
  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  SerializeIF::Endianness streamEndianness) const override {
    ReturnValue_t result =
        SerializeAdapter::serialize(&packetId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&packetSequenceControl, buffer, size, maxSize,
                                         streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (failureSubtype == tcverif::PROGRESS_FAILURE) {
      result = SerializeAdapter::serialize(&stepNumber, buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
    }
    result = SerializeAdapter::serialize(&errorCode, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&errorParameter1, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }

    result = SerializeAdapter::serialize(&errorParameter2, buffer, size, maxSize, streamEndianness);
    return result;
  }

  virtual size_t getSerializedSize() const {
    size_t size = 0;
    size += SerializeAdapter::getSerializedSize(&packetId);
    size += sizeof(packetSequenceControl);
    if (failureSubtype == tcverif::PROGRESS_FAILURE) {
      size += SerializeAdapter::getSerializedSize(&stepNumber);
    }
    size += SerializeAdapter::getSerializedSize(&errorCode);
    size += SerializeAdapter::getSerializedSize(&errorParameter1);
    size += SerializeAdapter::getSerializedSize(&errorParameter2);
    return size;
  }

  /**
   * Deserialization is not allowed for a report.
   * @param buffer
   * @param size
   * @param bigEndian
   * @return
   */
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            SerializeIF::Endianness streamEndianness) override {
    return returnvalue::FAILED;
  }

 private:
  uint16_t packetId;               //!< [EXPORT] : [COMMENT] Packet ID of respective Telecommand
  uint16_t packetSequenceControl;  //!< [EXPORT] : [COMMENT] Packet SSC of respective Telecommand
  uint8_t stepNumber;              //!< [EXPORT] : [OPTIONAL][SUBSERVICE] 6
  ReturnValue_t errorCode;  //!< [EXPORT] : [COMMENT] Error code which can be looked up in generated
                            //!< error code file
  uint32_t errorParameter1;
  uint32_t errorParameter2;
  const uint8_t failureSubtype;  //!< [EXPORT] : [IGNORE]
};

/**
 * @brief       Subservices 1, 3, 5, 7
 * @ingroup     spacepackets
 */
class SuccessReport : public SerializeIF {  //!< [EXPORT] : [SUBSERVICE] 1, 3, 5, 7
 public:
  SuccessReport(uint8_t subtype_, uint16_t packetId_, uint16_t packetSequenceControl_,
                uint8_t stepNumber_)
      : packetId(packetId_),
        packetSequenceControl(packetSequenceControl_),
        stepNumber(stepNumber_),
        subtype(subtype_) {}

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  SerializeIF::Endianness streamEndianness) const override {
    ReturnValue_t result =
        SerializeAdapter::serialize(&packetId, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&packetSequenceControl, buffer, size, maxSize,
                                         streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (subtype == tcverif::PROGRESS_SUCCESS) {
      result = SerializeAdapter::serialize(&stepNumber, buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
    }
    return result;
  }

  virtual size_t getSerializedSize() const override {
    size_t size = 0;
    size += SerializeAdapter::getSerializedSize(&packetId);
    size += sizeof(packetSequenceControl);
    if (subtype == tcverif::PROGRESS_SUCCESS) {
      size += SerializeAdapter::getSerializedSize(&stepNumber);
    }
    return size;
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            SerializeIF::Endianness streamEndianness) override {
    return returnvalue::FAILED;
  }

 private:
  uint16_t packetId;               //!< [EXPORT] : [COMMENT] Packet ID of respective Telecommand
  uint16_t packetSequenceControl;  //!< [EXPORT] : [COMMENT] Packet SSC of respective Telecommand
  uint8_t stepNumber;              //!< [EXPORT] : [OPTIONAL][SUBSERVICE] 6
  const uint8_t subtype;           //!< [EXPORT] : [IGNORE]
};

#endif /* MISSION_PUS_SERVICEPACKETS_SERVICE1PACKETS_H_ */
