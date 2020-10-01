#ifndef FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_
#define FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_

#include "ArrayList.h"
#include <cstring>
#include <set>
/**
 * @brief A "Map" which allows multiple entries of the same key.
 * @details
 * Same keys are ordered by KEY_COMPARE function which is std::less<key_t> > by default.
 *
 * It uses the ArrayList, so technically this is not a real map, it is an array of pairs
 * of type key_t, T. It is ordered by key_t as FixedMap but allows same keys.
 *
 * Its of fixed size so no allocations are performed after the construction.
 *
 * The maximum size is given as first parameter of the constructor.
 *
 * It provides an iterator to do list iterations.
 *
 * The type T must have a copy constructor if it is not trivial copy-able.
 *
 * @warning Iterators return a non-const key_t in the pair.
 * @warning A User is not allowed to change the key, otherwise the map is corrupted.
 *
 * \ingroup container
 */
template<typename key_t, typename T, typename KEY_COMPARE = std::less<key_t>>
class FixedOrderedMultimap {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIXED_MULTIMAP;
	static const ReturnValue_t MAP_FULL = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t KEY_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x02);

	/***
	 * Constructor which needs a size_t for the maximum allowed size
	 *
	 * Can not be resized during runtime
	 *
	 * Allocates memory at construction
	 * @param maxSize size_t of Maximum allowed size
	 */
	FixedOrderedMultimap(size_t maxSize) :
		theMap(maxSize), _size(0) {
	}
	/***
	 * Virtual destructor frees Memory by deleting its member
	 */
	virtual ~FixedOrderedMultimap() {
	}

	/***
	 * Special iterator for FixedOrderedMultimap
	 */
	class Iterator: public ArrayList<std::pair<key_t, T>, size_t>::Iterator {
	public:
		Iterator() :
			ArrayList<std::pair<key_t, T>, size_t>::Iterator() {
		}

		Iterator(std::pair<key_t, T> *pair) :
			ArrayList<std::pair<key_t, T>, size_t>::Iterator(pair) {
		}
	};

	friend bool operator==(const typename FixedOrderedMultimap::Iterator& lhs,
			const typename FixedOrderedMultimap::Iterator& rhs) {
		return (lhs.value == rhs.value);
	}

	friend bool operator!=(const typename FixedOrderedMultimap::Iterator& lhs,
			const typename FixedOrderedMultimap::Iterator& rhs) {
		return not (lhs.value == rhs.value);
	}

	/***
	 * Returns an iterator on the first element
	 * @return Iterator pointing to first element
	 */
	Iterator begin() const {
		return Iterator(&theMap[0]);
	}

	/**
	 * Returns an iterator pointing to one element past the end
	 * @return Iterator pointing to one element past the end
	 */
	Iterator end() const {
		return Iterator(&theMap[_size]);
	}

	/***
	 * Returns the current size of the map (not maximum size!)
	 * @return Current size
	 */
	size_t size() const {
		return _size;
	}

	/***
	 * Used to insert a key and value separately.
	 *
	 * @param[in] key Key of the new element
	 * @param[in] value Value of the new element
	 * @param[in/out] (optional) storedValue On success this points to the new value, otherwise a nullptr
	 * @return RETURN_OK if insert was successful, MAP_FULL if no space is available
	 */
	ReturnValue_t insert(key_t key, T value, Iterator *storedValue = nullptr) {
		if (_size == theMap.maxSize()) {
			return MAP_FULL;
		}
		size_t position = findNicePlace(key);
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

	/***
	 * Used to insert new pair instead of single values
	 *
	 * @param pair Pair to be inserted
	 * @return RETURN_OK if insert was successful, MAP_FULL if no space is available
	 */
	ReturnValue_t insert(std::pair<key_t, T> pair) {
		return insert(pair.first, pair.second);
	}

	/***
	 * Can be used to check if a certain key is in the map
	 * @param key Key to be checked
	 * @return RETURN_OK if the key exists KEY_DOES_NOT_EXIST otherwise
	 */
	ReturnValue_t exists(key_t key) const {
		ReturnValue_t result = KEY_DOES_NOT_EXIST;
		if (findFirstIndex(key) < _size) {
			result = HasReturnvaluesIF::RETURN_OK;
		}
		return result;
	}

	/***
	 * Used to delete the element in the iterator
	 *
	 * The iterator will point to the element before or begin(),
	 *  but never to one element in front of the map.
	 *
	 * @warning The iterator needs to be valid and dereferenceable
	 * @param[in/out] iter Pointer to iterator to the element that needs to be ereased
	 * @return RETURN_OK if erased, KEY_DOES_NOT_EXIST if the there is no element like this
	 */
	ReturnValue_t erase(Iterator *iter) {
		size_t i;
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

	/***
	 * Used to erase by key
	 * @param key Key to be erased
	 * @return RETURN_OK if erased, KEY_DOES_NOT_EXIST if the there is no element like this
	 */
	ReturnValue_t erase(key_t key) {
		size_t i;
		if ((i = findFirstIndex(key)) >= _size) {
			return KEY_DOES_NOT_EXIST;
		}
		do {
			removeFromPosition(i);
			i = findFirstIndex(key, i);
		} while (i < _size);
		return HasReturnvaluesIF::RETURN_OK;
	}

	/***
	 * Find returns the first appearance of the key
	 *
	 * If the key does not exist, it points to end()
	 *
	 * @param key Key to search for
	 * @return Iterator pointing to the first entry of key
	 */
	Iterator find(key_t key) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return end();
		}
		return Iterator(&theMap[findFirstIndex(key)]);
	}

	/***
	 * Finds first entry of the given key and returns a
	 * pointer to the value
	 *
	 * @param key Key to search for
	 * @param value Found value
	 * @return RETURN_OK if it points to the value,
	 * KEY_DOES_NOT_EXIST if the key is not in the map
	 */
	ReturnValue_t find(key_t key, T **value) const {
		ReturnValue_t result = exists(key);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		*value = &theMap[findFirstIndex(key)].second;
		return HasReturnvaluesIF::RETURN_OK;
	}
	/**
	 * Clears the map, does not deallocate any memory
	 */
	void clear() {
		_size = 0;
	}

	/**
	 * Returns the maximum size of the map
	 * @return Maximum size of the map
	 */
	size_t maxSize() const {
		return theMap.maxSize();
	}

private:
	typedef KEY_COMPARE compare;
	compare myComp;
	ArrayList<std::pair<key_t, T>, size_t> theMap;
	size_t _size;

	size_t findFirstIndex(key_t key, size_t startAt = 0) const {
		if (startAt >= _size) {
			return startAt + 1;
		}
		size_t i = startAt;
		for (i = startAt; i < _size; ++i) {
			if (theMap[i].first == key) {
				return i;
			}
		}
		return i;
	}

	size_t findNicePlace(key_t key) const {
		size_t i = 0;
		for (i = 0; i < _size; ++i) {
			if (myComp(key, theMap[i].first)) {
				return i;
			}
		}
		return i;
	}

	void removeFromPosition(size_t position) {
		if (_size <= position) {
			return;
		}
		memmove(static_cast<void*>(&theMap[position]), static_cast<void*>(&theMap[position + 1]),
				(_size - position - 1) * sizeof(std::pair<key_t,T>));
		--_size;
	}
};

#endif /* FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_ */
