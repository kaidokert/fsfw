#ifndef FRAMEWORK_CONTAINER_HYBRIDITERATOR_H_
#define FRAMEWORK_CONTAINER_HYBRIDITERATOR_H_

#include "ArrayList.h"
#include "SinglyLinkedList.h"

template <typename T, typename count_t = uint8_t>
class HybridIterator : public LinkedElement<T>::Iterator, public ArrayList<T, count_t>::Iterator {
 public:
  HybridIterator() {}

  HybridIterator(typename LinkedElement<T>::Iterator *iter)
      : LinkedElement<T>::Iterator(*iter), linked(true) {
    if (iter != nullptr) {
      value = iter->value;
    }
  }

  HybridIterator(LinkedElement<T> *start) : LinkedElement<T>::Iterator(start), linked(true) {
    if (start != nullptr) {
      value = start->value;
    }
  }

  HybridIterator(typename ArrayList<T, count_t>::Iterator start,
                 typename ArrayList<T, count_t>::Iterator end)
      : ArrayList<T, count_t>::Iterator(start), value(start.value), linked(false), end(end.value) {
    if (value == this->end) {
      value = nullptr;
    }
  }

  HybridIterator(T *firstElement, T *lastElement)
      : ArrayList<T, count_t>::Iterator(firstElement),
        value(firstElement),
        linked(false),
        end(++lastElement) {
    if (value == end) {
      value = NULL;
    }
  }

  HybridIterator &operator++() {
    if (linked) {
      LinkedElement<T>::Iterator::operator++();
      if (LinkedElement<T>::Iterator::value != nullptr) {
        value = LinkedElement<T>::Iterator::value->value;
      } else {
        value = nullptr;
      }
    } else {
      ArrayList<T, count_t>::Iterator::operator++();
      value = ArrayList<T, count_t>::Iterator::value;

      if (value == end) {
        value = nullptr;
      }
    }
    return *this;
  }

  HybridIterator operator++(int) {
    HybridIterator tmp(*this);
    operator++();
    return tmp;
  }

  bool operator==(const HybridIterator &other) const { return value == other.value; }

  bool operator!=(const HybridIterator &other) const { return !(*this == other); }

  T operator*() { return *value; }

  T *operator->() { return value; }

  T *value = nullptr;

 private:
  bool linked = false;
  T *end = nullptr;
};

#endif /* FRAMEWORK_CONTAINER_HYBRIDITERATOR_H_ */
