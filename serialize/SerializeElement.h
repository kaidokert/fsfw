#ifndef SERIALIZEELEMENT_H_
#define SERIALIZEELEMENT_H_

#include <framework/container/SinglyLinkedList.h>
#include <framework/serialize/SerializeAdapter.h>
#include <utility>

/**
 * @brief This class is used to mark datatypes for serialization with the
 *        SerialLinkedListAdapter
 * @details
 * Used by declaring any arbitrary datatype with SerializeElement<T> myVariable,
 * inside a SerialLinkedListAdapter implementation and setting the sequence
 * of objects with setNext() and setStart().
 * Serilization and Deserialization is then performed automatically in
 * specified sequence order.
 * @ingroup serialize
 */
template<typename T>
class SerializeElement : public SerializeIF, public LinkedElement<SerializeIF> {
public:
	/**
	 * Arguments are forwarded to the element datatype constructor
	 * @param args
	 */
	template<typename... Args>
	SerializeElement(Args... args) : LinkedElement<SerializeIF>(this), entry(std::forward<Args>(args)...) {

	}
	SerializeElement() : LinkedElement<SerializeIF>(this) {
	}

	T entry;

	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		return SerializeAdapter<T>::serialize(&entry, buffer, size, max_size, bigEndian);
	}

	size_t getSerializedSize() const {
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
