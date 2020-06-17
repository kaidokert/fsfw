#ifndef FRAMEWORK_CONTAINER_SINGLYLINKEDLIST_H_
#define FRAMEWORK_CONTAINER_SINGLYLINKEDLIST_H_

#include <cstddef>
#include <cstdint>

/**
 * @brief	Linked list data structure,
 * 			each entry has a pointer to the next entry (singly)
 * @ingroup container
 */
template<typename T>
class LinkedElement {
public:
	T *value;
	class Iterator {
	public:
		LinkedElement<T> *value = nullptr;
		Iterator() {}

		Iterator(LinkedElement<T> *element) :
				value(element) {
		}

		Iterator& operator++() {
			value = value->getNext();
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this);
			operator++();
			return tmp;
		}

		bool operator==(Iterator other) {
			return value == other.value;
		}

		bool operator!=(Iterator other) {
			return !(*this == other);
		}
		T *operator->() {
			return value->value;
		}
	};

	LinkedElement(T* setElement, LinkedElement<T>* setNext = nullptr):
	       value(setElement), next(setNext) {}

	virtual ~LinkedElement(){}

	virtual LinkedElement* getNext() const {
		return next;
	}

	virtual void setNext(LinkedElement* next) {
		this->next = next;
	}

	virtual void setEnd() {
	    this->next = nullptr;
	}

	LinkedElement* begin() {
		return this;
	}
	LinkedElement* end() {
		return nullptr;
	}
private:
	LinkedElement *next;
};

template<typename T>
class SinglyLinkedList {
public:
    using ElementIterator = typename LinkedElement<T>::Iterator;

	SinglyLinkedList() {}

	SinglyLinkedList(ElementIterator start) :
			start(start.value) {}

	SinglyLinkedList(LinkedElement<T>* startElement) :
			start(startElement) {}

	ElementIterator begin() const {
		return ElementIterator::Iterator(start);
	}
	typename ElementIterator::Iterator end() const {
		return ElementIterator::Iterator();
	}

	uint32_t getSize() const {
		uint32_t size = 0;
		LinkedElement<T> *element = start;
		while (element != NULL) {
			size++;
			element = element->getNext();
		}
		return size;
	}
	void setStart(LinkedElement<T>* setStart) {
		start = setStart;
	}

    void setEnd(LinkedElement<T>* setEnd) {
        setEnd->setEnd();
    }

    // SHOULDDO: Insertion operation ?

protected:
	LinkedElement<T> *start = nullptr;
};

#endif /* SINGLYLINKEDLIST_H_ */
