#ifndef FSFW_SUBSYSTEM_MODES_MODEDEFINITIONS_H_
#define FSFW_SUBSYSTEM_MODES_MODEDEFINITIONS_H_

#include "../../modes/HasModesIF.h"
#include "../../objectmanager/SystemObjectIF.h"
#include "../../serialize/SerialLinkedListAdapter.h"
#include "../../serialize/SerializeIF.h"

class ModeListEntry : public SerializeIF, public LinkedElement<ModeListEntry> {
 public:
  ModeListEntry() : LinkedElement<ModeListEntry>(this) {}

  uint32_t value1 = 0;
  uint32_t value2 = 0;
  uint8_t value3 = 0;
  uint8_t value4 = 0;

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const {
    ReturnValue_t result;

    result = SerializeAdapter::serialize(&value1, buffer, size, maxSize, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&value2, buffer, size, maxSize, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&value3, buffer, size, maxSize, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }

    result = SerializeAdapter::serialize(&value4, buffer, size, maxSize, streamEndianness);

    return result;
  }

  virtual size_t getSerializedSize() const {
    return sizeof(value1) + sizeof(value2) + sizeof(value3) + sizeof(value4);
  }

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) {
    ReturnValue_t result;

    result = SerializeAdapter::deSerialize(&value1, buffer, size, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&value2, buffer, size, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&value3, buffer, size, streamEndianness);

    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&value4, buffer, size, streamEndianness);

    return result;
  }

  // for Sequences
  Mode_t getTableId() const { return value1; }

  void setTableId(Mode_t tableId) { this->value1 = tableId; }

  uint8_t getWaitSeconds() const { return value2; }

  void setWaitSeconds(uint8_t waitSeconds) { this->value2 = waitSeconds; }

  bool checkSuccess() const { return value3 == 1; }

  void setCheckSuccess(bool checkSuccess) { this->value3 = checkSuccess; }

  // for Tables
  object_id_t getObject() const { return value1; }

  void setObject(object_id_t object) { this->value1 = object; }

  Mode_t getMode() const { return value2; }

  void setMode(Mode_t mode) { this->value2 = mode; }

  Submode_t getSubmode() const { return value3; }

  void setSubmode(Submode_t submode) { this->value3 = submode; }

  bool inheritSubmode() const { return value4 == 1; }

  void setInheritSubmode(bool inherit) {
    if (inherit) {
      value4 = 1;
    } else {
      value4 = 0;
    }
  }

  bool operator==(ModeListEntry other) {
    return ((value1 == other.value1) && (value2 == other.value2) && (value3 == other.value3));
  }
};

#endif /* FSFW_SUBSYSTEM_MODES_MODEDEFINITIONS_H_ */
