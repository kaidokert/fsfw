#ifndef FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_
#define FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_

#include <framework/container/ArrayList.h>
#include <cstring>
#include <set>

/**
 * @brief   Map implementation which allows entries with identical keys
 * @details
 * Performs no dynamic memory allocation except on initialization.
 * Uses an ArrayList as the underlying container and thus has a linear
 * complexity O(n). As long as the number of entries remains low, this
 * should not be an issue.
 * The number of insertion and deletion operation should be minimized
 * as those incur exensive memory move operations (the underlying container
 * is not node based).
 * @ingroup container
 */
template<typename key_t, typename T, typename KEY_COMPARE = std::less<key_t>>
class FixedOrderedMultimap {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIXED_MULTIMAP;
	static const ReturnValue_t KEY_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t MAP_FULL = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t KEY_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x03);

	/**
	 * Initializes the ordered multimap with a fixed maximum size.
	 * @param maxSize
	 */
    FixedOrderedMultimap(size_t maxSize);

    virtual ~FixedOrderedMultimap() {}

    class Iterator: public ArrayList<std::pair<key_t, T>, uint32_t>::Iterator {
    public:
        /** Returns an iterator to nullptr */
        Iterator();
        /** Initializes iterator to given entry */
        Iterator(std::pair<key_t, T> *pair);
        /** Dereference operator can be used to get value */
        T operator*();
        /** Arrow operator can be used to get pointer to value */
        T *operator->();
    };

    /** Iterator to start of map */
    Iterator begin() const;
    /** Iterator to end of map */
    Iterator end() const;
    /** Current (variable) size of the map */
    size_t size() const;

    /**
     * Insert a key/value pair inside the map. An iterator to the stored
     * value might be returned optionally.
     * @param key
     * @param value
     * @param storedValue
     * @return
     */
    ReturnValue_t insert(key_t key, T value, Iterator *storedValue = nullptr);
    /**
     * Insert a given std::pair<key, value>
     * @param pair
     * @return
     */
    ReturnValue_t insert(std::pair<key_t, T> pair);
    /**
     * Checks existence of key in map.
     * @param key
     * @return
     * - @c KEY_DOES_NOT_EXIST if key does not exists.
     * - @c RETURN_OK otherwise.
     */
    ReturnValue_t exists(key_t key) const;

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

    size_t maxSize() const {
        return theMap.maxSize();
    }

private:
	typedef KEY_COMPARE compare;
	compare myComp;
	ArrayList<std::pair<key_t, T>, uint32_t> theMap;
	size_t _size;

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
		memmove(&theMap[position], &theMap[position + 1],
				(_size - position - 1) * sizeof(std::pair<key_t,T>));
		--_size;
	}
};

#include <framework/container/FixedOrderedMultimap.tpp>

#endif /* FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_H_ */
