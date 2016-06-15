/*
 * SerializeElement.h
 *
 *  Created on: 24.03.2014
 *      Author: baetz
 */

#ifndef SERIALIZEELEMENT_H_
#define SERIALIZEELEMENT_H_

#include <framework/container/SinglyLinkedList.h>
#include <framework/serialize/SerializeAdapter.h>
#include <utility>

template<typename T>
class SerializeElement : public SerializeIF, public LinkedElement<SerializeIF> {
public:
	template<typename... Args>
	SerializeElement(Args... args) : LinkedElement<SerializeIF>(this), entry(std::forward<Args>(args)...) {

	}
	SerializeElement() : LinkedElement<SerializeIF>(this) {
	}
	T entry;
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerializeAdapter<T>::serialize(&entry, buffer, size, max_size, bigEndian);
	}

	uint32_t getSerializedSize() const {
		return SerializeAdapter<T>::getSerializedSize(&entry);
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerializeAdapter<T>::deSerialize(&entry, buffer, size, bigEndian);
	}
	operator T() {
		return entry;
	}

	SerializeElement<T> &operator=(T newValue) {
		entry = newValue;
		return *this;
	}
	T *operator->() {
		return &entry;
	}
};



#endif /* SERIALIZEELEMENT_H_ */
