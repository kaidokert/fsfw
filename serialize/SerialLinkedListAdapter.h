/**
 * @file	SerialLinkedListAdapter.h
 * @brief	This file defines the SerialLinkedListAdapter class.
 * @date	22.07.2014
 * @author	baetz
 */
#ifndef SERIALLINKEDLISTADAPTER_H_
#define SERIALLINKEDLISTADAPTER_H_

#include <framework/container/SinglyLinkedList.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerializeIF.h>
//This is where we need the SerializeAdapter!

 /**
 * @brief  	Implement the conversion of object data to data streams
 * 		   	or vice-versa, using linked lists.
 * @details
 * An alternative to the AutoSerializeAdapter functions
 *   - All object members with a datatype are declared as SerializeElement<element_type>
 * 	   members inside the class implementing this adapter.
 *   - The element type can also be a SerialBufferAdapter to de-/serialize buffers,
 *     with a known size, where the size can also be serialized
 *   - The element type can also be a SerialFixedArrayListAdapter to de-/serialize buffers
 *     with a size header, which is scanned automatically
 *
 * The sequence of objects is defined in the constructor by using
 * the setStart and setNext functions.
 *
 * - The serialization process is done by instantiating the class and
 *   calling serializ after all SerializeElement entries have been set by
 *   using the constructor or setter functions. An additional size variable can be supplied
 *   which is calculated/incremented automatically
 * - The deserialization process is done by instantiating the class and supplying
 *   a buffer with the data which is converted into an object. The size of
 *   data to serialize can be supplied and is decremented in the function
 *
 * @ingroup serialize
 */
template<typename T, typename count_t = uint8_t>
class SerialLinkedListAdapter: public SinglyLinkedList<T>, public SerializeIF {
public:
	SerialLinkedListAdapter(typename LinkedElement<T>::Iterator start,
			bool printCount = false) :
			SinglyLinkedList<T>(start), printCount(printCount) {
	}
	SerialLinkedListAdapter(LinkedElement<T>* first, bool printCount = false) :
			SinglyLinkedList<T>(first), printCount(printCount) {

	}
	SerialLinkedListAdapter(bool printCount = false) :
			SinglyLinkedList<T>(), printCount(printCount) {
	}

	/**
	 * Serialize object implementing this adapter into the supplied buffer
	 * and calculate the serialized size
	 * @param buffer [out] Object is serialized into this buffer. Note that the buffer pointer
	 *               *buffer is incremented automatically inside the respective serialize functions
	 * @param size [out] Calculated serialized size. Don't forget to set to 0.
	 * @param max_size
	 * @param bigEndian Specify endianness
	 * @return
	 */
	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		if (printCount) {
			count_t mySize = SinglyLinkedList<T>::getSize();
			ReturnValue_t result = SerializeAdapter<count_t>::serialize(&mySize,
					buffer, size, max_size, bigEndian);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
		return serialize(SinglyLinkedList<T>::start, buffer, size, max_size,
				bigEndian);
	}

	static ReturnValue_t serialize(const LinkedElement<T>* element,
			uint8_t** buffer, size_t* size, const size_t max_size,
			bool bigEndian) {
		ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (element != NULL)) {
			result = element->value->serialize(buffer, size, max_size,
					bigEndian);
			element = element->getNext();
		}
		return result;
	}
	virtual uint32_t getSerializedSize() const {
		if (printCount) {
			return SerialLinkedListAdapter<T>::getSerializedSize()
					+ sizeof(count_t);
		} else {
			return getSerializedSize(SinglyLinkedList<T>::start);
		}
	}
	static uint32_t getSerializedSize(const LinkedElement<T> *element) {
		uint32_t size = 0;
		while (element != NULL) {
			size += element->value->getSerializedSize();
			element = element->getNext();
		}
		return size;
	}

	/**
	 * Deserialize supplied buffer with supplied size into object implementing this adapter
	 * @param buffer
	 * @param size Decremented in respective deSerialize functions automatically
	 * @param bigEndian Specify endianness
	 * @return
	 */
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return deSerialize(SinglyLinkedList<T>::start, buffer, size, bigEndian);
	}

	static ReturnValue_t deSerialize(LinkedElement<T>* element,
			const uint8_t** buffer, int32_t* size, bool bigEndian) {
		ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (element != NULL)) {
			result = element->value->deSerialize(buffer, size, bigEndian);
			element = element->getNext();
		}
		return result;
	}

	bool printCount;

};

#endif /* SERIALLINKEDLISTADAPTER_H_ */
