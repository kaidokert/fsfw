#ifndef FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_
#define FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_

#include "ArrayList.h"
#include <cstring>
#include <set>
/**
 * \ingroup container
 */
template<typename key_t, typename T, typename KEY_COMPARE = std::less<key_t>>
class FixedOrderedMultimap {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIXED_MULTIMAP;
	static const ReturnValue_t MAP_FULL = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t KEY_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x02);

private:
	typedef KEY_COMPARE compare;
	compare myComp;
	ArrayList<std::pair<key_t, T>, uint32_t> theMap;
	uint32_t _size;

	uint32_t findFirstIndex(key_t key, uint32_t startAt = 0) const {
		if (startAt >= _size) {
			return startAt + 1;
		}
		uint32_t i = startAt;
		for (i = startAt; i < _size; ++i) {
			if (theMap[i].first == key) {
				return i;
			}
		}
		return i;
	}

	uint32_t findNicePlace(key_t key) const {
		uint32_t i = 0;
		for (i = 0; i < _size; ++i) {
			if (myComp(key, theMap[i].first)) {
				return i;
			}
		}
		return i;
	}

	void removeFromPosition(uint32_t position) {
		if (_size <= position) {
			return;
		}
		memmove(static_cast<void*>(&theMap[position]), static_cast<void*>(&theMap[position + 1]),
				(_size - position - 1) * sizeof(std::pair<key_t,T>));
		--_size;
	}
public:
	FixedOrderedMultimap(uint32_t maxSize) :
			theMap(maxSize), _size(0) {
	}
	virtual ~FixedOrderedMultimap() {
	}

	class Iterator: public ArrayList<std::pair<key_t, T>, uint32_t>::Iterator {
	public:
		Iterator() :
				ArrayList<std::pair<key_t, T>, uint32_t>::Iterator() {
		}

		Iterator(std::pair<key_t, T> *pair) :
				ArrayList<std::pair<key_t, T>, uint32_t>::Iterator(pair) {
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

	ReturnValue_t insert(key_t key, T value, Iterator *storedValue = nullptr) {
		if (_size == theMap.maxSize()) {
			return MAP_FULL;
		}
		uint32_t position = findNicePlace(key);
		memmove(static_cast<void*>(&theMap[position + 1]),static_cast<void*>(&theMap[position]),
				(_size - position) * sizeof(std::pair<key_t,T>));
		theMap[position].first = key;
		theMap[position].second = value;
		++_size;
		if (storedValue != nullptr) {
			*storedValue = Iterator(&theMap[position]);
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t insert(std::pair<key_t, T> pair) {
		return insert(pair.fist, pair.second);
	}

	ReturnValue_t exists(key_t key) const {
		ReturnValue_t result = KEY_DOES_NOT_EXIST;
		if (findFirstIndex(key) < _size) {
			result = HasReturnvaluesIF::RETURN_OK;
		}
		return result;
	}

	ReturnValue_t erase(Iterator *iter) {
		uint32_t i;
		if ((i = findFirstIndex((*iter).value->first)) >= _size) {
			return KEY_DOES_NOT_EXIST;
		}
		removeFromPosition(i);
		if (*iter != begin()) {
			(*iter)--;
		} else {
			*iter = begin();
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	ReturnValue_t erase(key_t key) {
		uint32_t i;
		if ((i = findFirstIndex(key)) >= _size) {
			return KEY_DOES_NOT_EXIST;
		}
		do {
			removeFromPosition(i);
			i = findFirstIndex(key, i);
		} while (i < _size);
		return HasReturnvaluesIF::RETURN_OK;
	}

	Iterator find(key_t key) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return end();
		}
		return Iterator(&theMap[findFirstIndex(key)]);
	}

	ReturnValue_t find(key_t key, T **value) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		*value = &theMap[findFirstIndex(key)].second;
		return HasReturnvaluesIF::RETURN_OK;
	}

	void clear() {
		_size = 0;
	}

	uint32_t maxSize() const {
		return theMap.maxSize();
	}

};

#endif /* FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_ */
