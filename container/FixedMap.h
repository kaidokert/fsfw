#ifndef FIXEDMAP_H_
#define FIXEDMAP_H_

#include "../container/ArrayList.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include <utility>

/**
 * @brief 	Map implementation for maps with a pre-defined size.
 * @details Can be initialized with desired maximum size.
 * 	        Iterator is used to access <key,value> pair and
 * 	        iterate through map entries. Complexity O(n).
 * @ingroup container
 */
template<typename key_t, typename T>
class FixedMap: public SerializeIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIXED_MAP;
	static const ReturnValue_t KEY_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t MAP_FULL = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t KEY_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x03);

private:
	static const key_t EMPTY_SLOT = -1;
	ArrayList<std::pair<key_t, T>, uint32_t> theMap;
	uint32_t _size;

	uint32_t findIndex(key_t key) const {
		if (_size == 0) {
			return 1;
		}
		uint32_t i = 0;
		for (i = 0; i < _size; ++i) {
			if (theMap[i].first == key) {
				return i;
			}
		}
		return i;
	}
public:
	FixedMap(uint32_t maxSize) :
			theMap(maxSize), _size(0) {
	}

	class Iterator: public ArrayList<std::pair<key_t, T>, uint32_t>::Iterator {
	public:
		Iterator() :
				ArrayList<std::pair<key_t, T>, uint32_t>::Iterator() {
		}

		Iterator(std::pair<key_t, T> *pair) :
				ArrayList<std::pair<key_t, T>, uint32_t>::Iterator(pair) {
		}

		T operator*() {
			return ArrayList<std::pair<key_t, T>, uint32_t>::Iterator::value->second;
		}

		// -> operator overloaded, can be used to access value
		T *operator->() {
			return &ArrayList<std::pair<key_t, T>, uint32_t>::Iterator::value->second;
		}

		// Can be used to access the key of the iterator
		key_t first() {
			return ArrayList<std::pair<key_t, T>, uint32_t>::Iterator::value->first;
		}

		// Alternative to access value, similar to std::map implementation
		T second() {
			return ArrayList<std::pair<key_t, T>, uint32_t>::Iterator::value->second;
		}
	};



	Iterator begin() const {
		return Iterator(&theMap[0]);
	}

	Iterator end() const {
		return Iterator(&theMap[_size]);
	}

	uint32_t size() const {
		return _size;
	}

	ReturnValue_t insert(key_t key, T value, Iterator *storedValue = NULL) {
		if (exists(key) == HasReturnvaluesIF::RETURN_OK) {
			return FixedMap::KEY_ALREADY_EXISTS;
		}
		if (_size == theMap.maxSize()) {
			return FixedMap::MAP_FULL;
		}
		theMap[_size].first = key;
		theMap[_size].second = value;
		if (storedValue != NULL) {
			*storedValue = Iterator(&theMap[_size]);
		}
		++_size;
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t insert(std::pair<key_t, T> pair) {
		return insert(pair.first, pair.second);
	}

	ReturnValue_t exists(key_t key) const {
		ReturnValue_t result = KEY_DOES_NOT_EXIST;
		if (findIndex(key) < _size) {
			result = HasReturnvaluesIF::RETURN_OK;
		}
		return result;
	}

	ReturnValue_t erase(Iterator *iter) {
		uint32_t i;
		if ((i = findIndex((*iter).value->first)) >= _size) {
			return KEY_DOES_NOT_EXIST;
		}
		theMap[i] = theMap[_size - 1];
		--_size;
		--((*iter).value);
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t erase(key_t key) {
		uint32_t i;
		if ((i = findIndex(key)) >= _size) {
			return KEY_DOES_NOT_EXIST;
		}
		theMap[i] = theMap[_size - 1];
		--_size;
		return HasReturnvaluesIF::RETURN_OK;
	}

	T *findValue(key_t key) const {
		return &theMap[findIndex(key)].second;
	}

	Iterator find(key_t key) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return end();
		}
		return Iterator(&theMap[findIndex(key)]);
	}

	ReturnValue_t find(key_t key, T **value) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		*value = &theMap[findIndex(key)].second;
		return HasReturnvaluesIF::RETURN_OK;
	}

	void clear() {
		_size = 0;
	}

	uint32_t maxSize() const {
		return theMap.maxSize();
	}


	bool full() {
		if(_size == theMap.maxSize()) {
			return true;
		}
		else {
			return false;
		}
	}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		ReturnValue_t result = SerializeAdapter::serialize(&this->_size,
				buffer, size, maxSize, streamEndianness);
		uint32_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < this->_size)) {
			result = SerializeAdapter::serialize(&theMap[i].first, buffer,
					size, maxSize, streamEndianness);
			result = SerializeAdapter::serialize(&theMap[i].second, buffer, size,
					maxSize, streamEndianness);
			++i;
		}
		return result;
	}

	virtual size_t getSerializedSize() const {
		uint32_t printSize = sizeof(_size);
		uint32_t i = 0;

		for (i = 0; i < _size; ++i) {
			printSize += SerializeAdapter::getSerializedSize(
					&theMap[i].first);
			printSize += SerializeAdapter::getSerializedSize(&theMap[i].second);
		}

		return printSize;
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		ReturnValue_t result = SerializeAdapter::deSerialize(&this->_size,
				buffer, size, streamEndianness);
		if (this->_size > theMap.maxSize()) {
			return SerializeIF::TOO_MANY_ELEMENTS;
		}
		uint32_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < this->_size)) {
			result = SerializeAdapter::deSerialize(&theMap[i].first, buffer,
					size, streamEndianness);
			result = SerializeAdapter::deSerialize(&theMap[i].second, buffer, size,
					streamEndianness);
			++i;
		}
		return result;
	}

};

#endif /* FIXEDMAP_H_ */
