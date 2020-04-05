/**
 * @file	SerialArrayListAdapter.h
 * @brief	This file defines the SerialArrayListAdapter class.
 * @date	22.07.2014
 * @author	baetz
 */
#ifndef SERIALARRAYLISTADAPTER_H_
#define SERIALARRAYLISTADAPTER_H_

#include <framework/container/ArrayList.h>
#include <framework/serialize/SerializeIF.h>
#include <utility>

/**
 * Also serializes length field !
 * \ingroup serialize
 */
template<typename T, typename count_t = uint8_t>
class SerialArrayListAdapter : public SerializeIF {
public:
	SerialArrayListAdapter(ArrayList<T, count_t> *adaptee) : adaptee(adaptee) {
	}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		return serialize(adaptee, buffer, size, max_size, bigEndian);
	}

	static ReturnValue_t serialize(const ArrayList<T, count_t>* list,
			uint8_t** buffer, size_t* size, const size_t max_size,
			bool bigEndian) {
		// Serialize length field first
		ReturnValue_t result = SerializeAdapter<count_t>::serialize(&list->size,
				buffer, size, max_size, bigEndian);
		count_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < list->size)) {
			result = SerializeAdapter<T>::serialize(&list->entries[i], buffer, size,
					max_size, bigEndian);
			++i;
		}
		return result;
	}

	virtual size_t getSerializedSize() const {
		return getSerializedSize(adaptee);
	}

	static uint32_t getSerializedSize(const ArrayList<T, count_t>* list) {
		uint32_t printSize = sizeof(count_t);
		count_t i = 0;

		for (i = 0; i < list->size; ++i) {
			printSize += SerializeAdapter<T>::getSerializedSize(&list->entries[i]);
		}

		return printSize;
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return deSerialize(adaptee, buffer, size, bigEndian);
	}

	static ReturnValue_t deSerialize(ArrayList<T, count_t>* list, const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		count_t tempSize = 0;
		ReturnValue_t result = SerializeAdapter<count_t>::deSerialize(&tempSize,
				buffer, size, bigEndian);
		if (tempSize > list->maxSize()) {
			return SerializeIF::TOO_MANY_ELEMENTS;
		}
		list->size = tempSize;
		count_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < list->size)) {
			result = SerializeAdapter<T>::deSerialize(
					&list->front()[i], buffer, size,
					bigEndian);
			++i;
		}
		return result;
	}


	static void swapArrayListEndianness(ArrayList<T, count_t>* list) {
		list->swapArrayListEndianness();
	}
private:
	ArrayList<T, count_t> *adaptee;
};



#endif /* SERIALARRAYLISTADAPTER_H_ */
