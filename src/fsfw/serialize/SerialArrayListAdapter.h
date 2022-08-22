#ifndef FSFW_SERIALIZE_SERIALARRAYLISTADAPTER_H_
#define FSFW_SERIALIZE_SERIALARRAYLISTADAPTER_H_

#include <utility>

#include "../container/ArrayList.h"
#include "SerializeIF.h"

/**
 * Also serializes length field !
 * @author 	baetz
 * @ingroup serialize
 */
template <typename T, typename count_t = uint8_t>
class SerialArrayListAdapter : public SerializeIF {
 public:
  SerialArrayListAdapter(ArrayList<T, count_t>* adaptee) : adaptee(adaptee) {}

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const {
    return serialize(adaptee, buffer, size, maxSize, streamEndianness);
  }

  static ReturnValue_t serialize(const ArrayList<T, count_t>* list, uint8_t** buffer, size_t* size,
                                 size_t maxSize, Endianness streamEndianness) {
    ReturnValue_t result =
        SerializeAdapter::serialize(&list->size, buffer, size, maxSize, streamEndianness);
    count_t i = 0;
    while ((result == returnvalue::OK) && (i < list->size)) {
      result =
          SerializeAdapter::serialize(&list->entries[i], buffer, size, maxSize, streamEndianness);
      ++i;
    }
    return result;
  }

  virtual size_t getSerializedSize() const { return getSerializedSize(adaptee); }

  static uint32_t getSerializedSize(const ArrayList<T, count_t>* list) {
    uint32_t printSize = sizeof(count_t);
    count_t i = 0;

    for (i = 0; i < list->size; ++i) {
      printSize += SerializeAdapter::getSerializedSize(&list->entries[i]);
    }

    return printSize;
  }

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) {
    return deSerialize(adaptee, buffer, size, streamEndianness);
  }

  static ReturnValue_t deSerialize(ArrayList<T, count_t>* list, const uint8_t** buffer,
                                   size_t* size, Endianness streamEndianness) {
    count_t tempSize = 0;
    ReturnValue_t result = SerializeAdapter::deSerialize(&tempSize, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (tempSize > list->maxSize()) {
      return SerializeIF::TOO_MANY_ELEMENTS;
    }

    list->size = tempSize;
    count_t i = 0;
    while ((result == returnvalue::OK) && (i < list->size)) {
      result = SerializeAdapter::deSerialize(&list->front()[i], buffer, size, streamEndianness);
      ++i;
    }
    return result;
  }

 private:
  ArrayList<T, count_t>* adaptee;
};

#endif /* FSFW_SERIALIZE_SERIALARRAYLISTADAPTER_H_ */
