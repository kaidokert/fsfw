/*
 * PrivatePoolEntry.h
 *
 *  Created on: 13.03.2014
 *      Author: baetz
 */

#ifndef PRIVATEPOOLENTRY_H_
#define PRIVATEPOOLENTRY_H_

#include <framework/container/IsDerivedFrom.h>
#include <framework/container/SinglyLinkedList.h>
#include <framework/privatepool/PrivatePoolIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <utility>

template<class T, typename = void>
class PrivatePoolEntry: public PrivatePoolIF, public LinkedElement<PrivatePoolIF> {
public:
	PrivatePoolEntry() : LinkedElement<PrivatePoolIF>(this),
	address(0), element(0) {
	}
	PrivatePoolEntry(uint32_t address, T value) : LinkedElement<PrivatePoolIF>(this),
			address(address), element(value) {
	}

	uint32_t address;

	T element;

	operator T() {
		return element;
	}

	PrivatePoolEntry<T> &operator =(T value) {
		element = value;
		return *this;
	}
	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer) {
		return handleMemoryDump(address, size, dataPointer, NULL);
	}
	ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* dumpTarget ) {
		if (this->address != address) {
			return INVALID_ADDRESS;
		}
		if (size != sizeof(element)) {
			return INVALID_SIZE;
		}
		*dataPointer = (uint8_t*)&element;
		return POINTS_TO_VARIABLE;
	}
	ReturnValue_t setAddress(uint32_t* setAddress ) {
		address = *setAddress;
		return HasReturnvaluesIF::RETURN_OK;
	}
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerializeAdapter<T>::serialize(&element, buffer, size, max_size, bigEndian);
	}

	uint32_t getSerializedSize() const {
		return SerializeAdapter<T>::getSerializedSize(&element);
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerializeAdapter<T>::deSerialize(&element, buffer, size, bigEndian);

	}
};

template <class T>
class PrivatePoolEntry<T, typename enable_if<IsDerivedFrom<T, PrivatePoolIF>::Is>::type> : public LinkedElement<PrivatePoolIF>, public T {
public:
	//TODO: Have a look on how std::forward works.
	template<typename... Args>
	PrivatePoolEntry(Args... args) : LinkedElement<PrivatePoolIF>(this), T(std::forward<Args>(args)...) {

	}
	virtual ~PrivatePoolEntry() {

	}
};

#endif /* PRIVATEPOOLENTRY_H_ */
