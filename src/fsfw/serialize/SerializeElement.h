#ifndef FSFW_SERIALIZE_SERIALIZEELEMENT_H_
#define FSFW_SERIALIZE_SERIALIZEELEMENT_H_

#include <utility>

#include "../container/SinglyLinkedList.h"
#include "SerializeAdapter.h"

/**
 * @brief This class is used to mark datatypes for serialization with the
 *        SerialLinkedListAdapter
 * @details
 * Used by declaring any arbitrary datatype with SerializeElement<T> myVariable,
 * inside a SerialLinkedListAdapter implementation and setting the sequence
 * of objects with setNext() and setStart().
 * Serialization and Deserialization is then performed automatically in
 * specified sequence order.
 * @ingroup serialize
 */
template <typename T>
class SerializeElement : public SerializeIF, public LinkedElement<SerializeIF> {
 public:
  template <typename... Args>
  SerializeElement(Args... args)
      : LinkedElement<SerializeIF>(this), entry(std::forward<Args>(args)...) {}
  SerializeElement() : LinkedElement<SerializeIF>(this), entry() {}

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&entry, buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const override { return SerializeAdapter::getSerializedSize(&entry); }

  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&entry, buffer, size, streamEndianness);
  }

  operator T() { return entry; }

  SerializeElement<T> &operator=(T newValue) {
    entry = newValue;
    return *this;
  }

  T *operator->() { return &entry; }

  T entry;
};

#endif /* FSFW_SERIALIZE_SERIALIZEELEMENT_H_ */
