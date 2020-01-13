#ifndef FIXEDMAP_H_
#define FIXEDMAP_H_

#include <framework/container/ArrayList.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <utility>

/**
 * @brief 	Map implementation for maps with a pre-defined size.
 * @details Can be initialized with desired maximum size.
 * 	        Iterator is used to access <key,value> pair and
 * 	        iterate through map entries.
 * @ingroup container
 */
template<typename key_t, typename T>
class FixedMap: public SerializeIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIXED_MAP;
	static const ReturnValue_t KEY_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01); //!< P1: SID for HK packets
	static const ReturnValue_t MAP_FULL = MAKE_RETURN_CODE(0x02); //!< P1: SID for HK packets
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

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		ReturnValue_t result = SerializeAdapter<uint32_t>::serialize(&this->_size,
				buffer, size, max_size, bigEndian);
		uint32_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < this->_size)) {
			result = SerializeAdapter<key_t>::serialize(&theMap[i].first, buffer,
					size, max_size, bigEndian);
			result = SerializeAdapter<T>::serialize(&theMap[i].second, buffer, size,
					max_size, bigEndian);
			++i;
		}
		return result;
	}

	virtual uint32_t getSerializedSize() const {
		uint32_t printSize = sizeof(_size);
		uint32_t i = 0;

		for (i = 0; i < _size; ++i) {
			printSize += SerializeAdapter<key_t>::getSerializedSize(
					&theMap[i].first);
			printSize += SerializeAdapter<T>::getSerializedSize(&theMap[i].second);
		}

		return printSize;
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		ReturnValue_t result = SerializeAdapter<uint32_t>::deSerialize(&this->_size,
				buffer, size, bigEndian);
		if (this->_size > theMap.maxSize()) {
			return SerializeIF::TOO_MANY_ELEMENTS;
		}
		uint32_t i = 0;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (i < this->_size)) {
			result = SerializeAdapter<key_t>::deSerialize(&theMap[i].first, buffer,
					size, bigEndian);
			result = SerializeAdapter<T>::deSerialize(&theMap[i].second, buffer, size,
					bigEndian);
			++i;
		}
		return result;
	}

};

#endif /* FIXEDMAP_H_ */
