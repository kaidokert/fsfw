#ifndef SERIALIZEELEMENT_H_
#define SERIALIZEELEMENT_H_

#include <framework/container/SinglyLinkedList.h>
#include <framework/serialize/SerializeAdapter.h>
#include <utility>

/**
 * \ingroup serialize
 */
template<typename T>
class SerializeElement: public SerializeIF, public LinkedElement<SerializeIF> {
public:
	template<typename ... Args>
	SerializeElement(Args ... args) :
			LinkedElement<SerializeIF>(this), entry(std::forward<Args>(args)...) {

	}
	SerializeElement() :
			LinkedElement<SerializeIF>(this) {
	}
	T entry;
	ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
			Endianness streamEndianness) const override {
		return SerializeAdapter::serialize(&entry, buffer, size, maxSize,
				streamEndianness);
	}

	uint32_t getSerializedSize() const override {
		return SerializeAdapter::getSerializedSize(&entry);
	}

	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) override {
		return SerializeAdapter::deSerialize(&entry, buffer, size,
				streamEndianness);
	}
	operator T() {
		return entry;
	}

	SerializeElement<T>& operator=(T newValue) {
		entry = newValue;
		return *this;
	}
	T* operator->() {
		return &entry;
	}
};

#endif /* SERIALIZEELEMENT_H_ */
