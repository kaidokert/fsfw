#ifndef HYBRIDITERATOR_H_
#define HYBRIDITERATOR_H_

#include <framework/container/ArrayList.h>
#include <framework/container/SinglyLinkedList.h>

template<typename T, typename count_t = uint8_t>
class HybridIterator: public LinkedElement<T>::Iterator,
		public ArrayList<T, count_t>::Iterator {
public:
	HybridIterator() :
			value(NULL), linked(NULL), end(NULL) {
	}

	HybridIterator(typename LinkedElement<T>::Iterator *iter) :
		LinkedElement<T>::Iterator(*iter), value(
					iter->value), linked(true), end(NULL) {

	}

	HybridIterator(LinkedElement<T> *start) :
		LinkedElement<T>::Iterator(start), value(
					start->value), linked(true), end(NULL) {

	}

	HybridIterator(typename ArrayList<T, count_t>::Iterator start,
			typename ArrayList<T, count_t>::Iterator end) :
			ArrayList<T, count_t>::Iterator(start), value(start.value), linked(
					false), end(end.value) {
		if (value == this->end) {
			value = NULL;
		}
	}

	HybridIterator(T *firstElement, T *lastElement) :
			ArrayList<T, count_t>::Iterator(firstElement), value(firstElement), linked(
					false), end(++lastElement) {
		if (value == end) {
			value = NULL;
		}
	}

	HybridIterator& operator++() {
		if (linked) {
			LinkedElement<T>::Iterator::operator++();
			if (LinkedElement<T>::Iterator::value != NULL) {
				value = LinkedElement<T>::Iterator::value->value;
			} else {
				value = NULL;
			}
		} else {
			ArrayList<T, count_t>::Iterator::operator++();
			value = ArrayList<T, count_t>::Iterator::value;

			if (value == end) {
				value = NULL;
			}
		}
		return *this;
	}

	HybridIterator operator++(int) {
		HybridIterator tmp(*this);
		operator++();
		return tmp;
	}

	bool operator==(HybridIterator other) {
		return value == other.value;
	}

	bool operator!=(HybridIterator other) {
		return !(*this == other);
	}

	T operator*() {
		return *value;
	}

	T *operator->() {
		return value;
	}

	T* value;

private:
	bool linked;
	T *end;
};

#endif /* HYBRIDITERATOR_H_ */
