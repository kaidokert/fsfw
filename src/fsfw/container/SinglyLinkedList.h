#ifndef FRAMEWORK_CONTAINER_SINGLYLINKEDLIST_H_
#define FRAMEWORK_CONTAINER_SINGLYLINKEDLIST_H_

#include <cstddef>
#include <cstdint>

/**
 * @brief    Linked list data structure,
 *             each entry has a pointer to the next entry (singly)
 * @ingroup container
 */
template <typename T>
class LinkedElement {
 public:
  T* value;
  class Iterator {
   public:
    LinkedElement<T>* value = nullptr;
    Iterator() {}

    Iterator(LinkedElement<T>* element) : value(element) {}

    Iterator& operator++() {
      value = value->getNext();
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(Iterator other) { return value == other.value; }

    bool operator!=(Iterator other) { return !(*this == other); }
    T* operator->() { return value->value; }
  };

  LinkedElement(T* setElement, LinkedElement<T>* setNext = nullptr)
      : value(setElement), next(setNext) {}

  virtual ~LinkedElement() {}

  virtual LinkedElement* getNext() const { return next; }

  virtual void setNext(LinkedElement* next) { this->next = next; }

  virtual void setEnd() { this->next = nullptr; }

  LinkedElement* begin() { return this; }
  LinkedElement* end() { return nullptr; }

 private:
  LinkedElement* next;
};

template <typename T>
class SinglyLinkedList {
 public:
  using ElementIterator = typename LinkedElement<T>::Iterator;

  SinglyLinkedList() {}

  SinglyLinkedList(ElementIterator start) : start(start.value) {}

  SinglyLinkedList(LinkedElement<T>* startElement) : start(startElement) {}

  ElementIterator begin() const { return ElementIterator::Iterator(start); }

  /** Returns iterator to nulltr */
  ElementIterator end() const { return ElementIterator::Iterator(); }

  /**
   * Returns last element in singly linked list.
   * @return
   */
  ElementIterator back() const {
    LinkedElement<T>* element = start;
    while (element->getNext() != nullptr) {
      element = element->getNext();
    }
    return ElementIterator::Iterator(element);
  }

  size_t getSize() const {
    size_t size = 0;
    LinkedElement<T>* element = start;
    while (element != nullptr) {
      size++;
      element = element->getNext();
    }
    return size;
  }
  void setStart(LinkedElement<T>* firstElement) { start = firstElement; }

  void setNext(LinkedElement<T>* currentElement, LinkedElement<T>* nextElement) {
    currentElement->setNext(nextElement);
  }

  void setLast(LinkedElement<T>* lastElement) { lastElement->setEnd(); }

  void insertElement(LinkedElement<T>* element, size_t position) {
    LinkedElement<T>* currentElement = start;
    for (size_t count = 0; count < position; count++) {
      if (currentElement == nullptr) {
        return;
      }
      currentElement = currentElement->getNext();
    }
    LinkedElement<T>* elementAfterCurrent = currentElement->next;
    currentElement->setNext(element);
    if (elementAfterCurrent != nullptr) {
      element->setNext(elementAfterCurrent);
    }
  }

  void insertBack(LinkedElement<T>* lastElement) { back().value->setNext(lastElement); }

 protected:
  LinkedElement<T>* start = nullptr;
};

#endif /* SINGLYLINKEDLIST_H_ */
