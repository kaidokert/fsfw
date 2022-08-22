#ifndef FSFW_SERIALIZE_SERIALLINKEDLISTADAPTER_H_
#define FSFW_SERIALIZE_SERIALLINKEDLISTADAPTER_H_

#include "../container/SinglyLinkedList.h"
#include "SerializeAdapter.h"
#include "SerializeElement.h"
#include "SerializeIF.h"

/**
 * @brief  	Implement the conversion of object data to data streams
 * 		   	or vice-versa, using linked lists.
 * @details
 * An alternative to the AutoSerializeAdapter functions
 *   - All object members with a datatype are declared as
 *     SerializeElement<element_type> members inside the class
 *     implementing this adapter.
 *   - The element type can also be a SerialBufferAdapter to
 *     de-/serialize buffers.
 *   - The element type can also be a SerialFixedArrayListAdapter to
 *     de-/serialize buffers with a size header, which is scanned automatically.
 *
 * The sequence of objects is defined in the constructor by using
 * the setStart and setNext functions.
 *
 * 1. The serialization process is done by instantiating the class and
 *    calling serialize after all SerializeElement entries have been set by
 *    using the constructor or setter functions. An additional size variable
 *    can be supplied which is calculated/incremented automatically.
 * 2. The deserialization process is done by instantiating the class and
 *    supplying a buffer with the data which is converted into an object.
 *    The size of data to serialize can be supplied and is
 *    decremented in the function. Range checking is done internally.
 * @author baetz
 * @ingroup serialize
 */
template <typename T, typename count_t = uint8_t>
class SerialLinkedListAdapter : public SinglyLinkedList<T>, public SerializeIF {
 public:
  SerialLinkedListAdapter(typename LinkedElement<T>::Iterator start, bool printCount = false)
      : SinglyLinkedList<T>(start), printCount(printCount) {}

  SerialLinkedListAdapter(LinkedElement<T>* first, bool printCount = false)
      : SinglyLinkedList<T>(first), printCount(printCount) {}

  SerialLinkedListAdapter(bool printCount = false)
      : SinglyLinkedList<T>(), printCount(printCount) {}

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const override {
    if (printCount) {
      count_t mySize = SinglyLinkedList<T>::getSize();
      ReturnValue_t result =
          SerializeAdapter::serialize(&mySize, buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
    }
    return serialize(SinglyLinkedList<T>::start, buffer, size, maxSize, streamEndianness);
  }

  static ReturnValue_t serialize(const LinkedElement<T>* element, uint8_t** buffer, size_t* size,
                                 size_t maxSize, Endianness streamEndianness) {
    ReturnValue_t result = returnvalue::OK;
    while ((result == returnvalue::OK) and (element != nullptr)) {
      result = element->value->serialize(buffer, size, maxSize, streamEndianness);
      element = element->getNext();
    }
    return result;
  }

  virtual size_t getSerializedSize() const override {
    if (printCount) {
      return SerialLinkedListAdapter<T>::getSerializedSize() + sizeof(count_t);
    } else {
      return getSerializedSize(SinglyLinkedList<T>::start);
    }
  }

  static size_t getSerializedSize(const LinkedElement<T>* element) {
    size_t size = 0;
    while (element != nullptr) {
      size += element->value->getSerializedSize();
      element = element->getNext();
    }
    return size;
  }

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) override {
    return deSerialize(SinglyLinkedList<T>::start, buffer, size, streamEndianness);
  }

  static ReturnValue_t deSerialize(LinkedElement<T>* element, const uint8_t** buffer, size_t* size,
                                   Endianness streamEndianness) {
    ReturnValue_t result = returnvalue::OK;
    while ((result == returnvalue::OK) and (element != nullptr)) {
      result = element->value->deSerialize(buffer, size, streamEndianness);
      element = element->getNext();
    }
    return result;
  }

  /**
   * Copying is forbidden by deleting the copy constructor and the copy
   * assignment operator because of the pointers to the linked list members.
   * Unless the child class implements an own copy constructor or
   * copy assignment operator, these operation will throw a compiler error.
   * @param
   */
  SerialLinkedListAdapter(const SerialLinkedListAdapter&) = delete;
  SerialLinkedListAdapter& operator=(const SerialLinkedListAdapter&) = delete;

  bool printCount;
};

#endif /* FSFW_SERIALIZE_SERIALLINKEDLISTADAPTER_H_ */
