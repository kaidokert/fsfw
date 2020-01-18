#ifndef SINGLYLINKEDLIST_H_
#define SINGLYLINKEDLIST_H_

#include <stddef.h>
#include <stdint.h>
/**
 * \ingroup container
 */
template<typename T>
class LinkedElement {
public:
	T *value;
	class Iterator {
	public:
		LinkedElement<T> *value;
		Iterator() :
				value(NULL) {

		}

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

	LinkedElement(T* setElement, LinkedElement<T>* setNext = NULL) : value(setElement),
		next(setNext) {
	}
	virtual ~LinkedElement(){

	}
	virtual LinkedElement* getNext() const {
		return next;
	}

	virtual void setNext(LinkedElement* next) {
		this->next = next;
	}

	void setEnd() {
		this->next = NULL;
	}

	LinkedElement* begin() {
		return this;
	}
	LinkedElement* end() {
		return NULL;
	}
private:
	LinkedElement *next;
};

template<typename T>
class SinglyLinkedList {
public:
	SinglyLinkedList() :
			start(NULL) {
	}

	SinglyLinkedList(typename LinkedElement<T>::Iterator start) :
			start(start.value) {
	}
	SinglyLinkedList(LinkedElement<T>* startElement) :
				start(startElement) {
		}
	typename LinkedElement<T>::Iterator begin() const {
		return LinkedElement<T>::Iterator::Iterator(start);
	}
	typename LinkedElement<T>::Iterator::Iterator end() const {
		return LinkedElement<T>::Iterator::Iterator();
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
protected:
	LinkedElement<T> *start;
};

#endif /* SINGLYLINKEDLIST_H_ */
