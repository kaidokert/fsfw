#ifndef FSFW_CONTAINER_FIXEDORDEREDMULTIMAP_H_
#define FSFW_CONTAINER_FIXEDORDEREDMULTIMAP_H_

#include <cstring>
#include <functional>

#include "ArrayList.h"

/**
 * @brief  An associative container which allows multiple entries of the same key.
 * @details
 * Same keys are ordered by KEY_COMPARE function which is std::less<key_t> > by default.
 *
 * It uses the ArrayList, so technically this is not a real map, it is an array of pairs
 * of type key_t, T. It is ordered by key_t as FixedMap but allows same keys. Thus it has a linear
 * complexity O(n). As long as the number of entries remains low, this
 * should not be an issue.
 * The number of insertion and deletion operation should be minimized
 * as those incur extensive memory move operations (the underlying container
 * is not node based).
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
template <typename key_t, typename T, typename KEY_COMPARE = std::less<key_t>>
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
  FixedOrderedMultimap(size_t maxSize) : theMap(maxSize), _size(0) {}

  /***
   * Virtual destructor frees Memory by deleting its member
   */
  virtual ~FixedOrderedMultimap() {}

  /***
   * Special iterator for FixedOrderedMultimap
   */
  class Iterator : public ArrayList<std::pair<key_t, T>, size_t>::Iterator {
   public:
    Iterator() : ArrayList<std::pair<key_t, T>, size_t>::Iterator() {}

    Iterator(std::pair<key_t, T>* pair) : ArrayList<std::pair<key_t, T>, size_t>::Iterator(pair) {}
  };

  /***
   * Returns an iterator pointing to the first element
   * @return Iterator pointing to first element
   */
  Iterator begin() const { return Iterator(&theMap[0]); }

  /**
   * Returns an iterator pointing to one element past the end
   * @return Iterator pointing to one element past the end
   */
  Iterator end() const { return Iterator(&theMap[_size]); }

  /***
   * Returns the current size of the map (not maximum size!)
   * @return Current size
   */
  size_t size() const { return _size; }

  /**
   * Clears the map, does not deallocate any memory
   */
  void clear() { _size = 0; }

  /**
   * Returns the maximum size of the map
   * @return Maximum size of the map
   */
  size_t maxSize() const { return theMap.maxSize(); }

  /***
   * Used to insert a key and value separately.
   *
   * @param[in] key Key of the new element
   * @param[in] value Value of the new element
   * @param[in/out] (optional) storedValue On success this points to the new value, otherwise a
   * nullptr
   * @return returnvalue::OK if insert was successful, MAP_FULL if no space is available
   */
  ReturnValue_t insert(key_t key, T value, Iterator* storedValue = nullptr);

  /***
   * Used to insert new pair instead of single values
   *
   * @param pair Pair to be inserted
   * @return returnvalue::OK if insert was successful, MAP_FULL if no space is available
   */
  ReturnValue_t insert(std::pair<key_t, T> pair);

  /***
   * Can be used to check if a certain key is in the map
   * @param key Key to be checked
   * @return returnvalue::OK if the key exists KEY_DOES_NOT_EXIST otherwise
   */
  ReturnValue_t exists(key_t key) const;

  /***
   * Used to delete the element in the iterator
   *
   * The iterator will point to the element before or begin(),
   *  but never to one element in front of the map.
   *
   * @warning The iterator needs to be valid and dereferenceable
   * @param[in/out] iter Pointer to iterator to the element that needs to be ereased
   * @return returnvalue::OK if erased, KEY_DOES_NOT_EXIST if the there is no element like this
   */
  ReturnValue_t erase(Iterator* iter);

  /***
   * Used to erase by key
   * @param key Key to be erased
   * @return returnvalue::OK if erased, KEY_DOES_NOT_EXIST if the there is no element like this
   */
  ReturnValue_t erase(key_t key);

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
    if (result != returnvalue::OK) {
      return end();
    }
    return Iterator(&theMap[findFirstIndex(key)]);
  };

  /***
   * Finds first entry of the given key and returns a
   * pointer to the value
   *
   * @param key Key to search for
   * @param value Found value
   * @return returnvalue::OK if it points to the value,
   * KEY_DOES_NOT_EXIST if the key is not in the map
   */
  ReturnValue_t find(key_t key, T** value) const;

  friend bool operator==(const typename FixedOrderedMultimap::Iterator& lhs,
                         const typename FixedOrderedMultimap::Iterator& rhs) {
    return (lhs.value == rhs.value);
  }

  friend bool operator!=(const typename FixedOrderedMultimap::Iterator& lhs,
                         const typename FixedOrderedMultimap::Iterator& rhs) {
    return not(lhs.value == rhs.value);
  }

 private:
  typedef KEY_COMPARE compare;
  compare myComp;
  ArrayList<std::pair<key_t, T>, size_t> theMap;
  size_t _size;

  size_t findFirstIndex(key_t key, size_t startAt = 0) const;

  size_t findNicePlace(key_t key) const;

  void removeFromPosition(size_t position);
};

#include "FixedOrderedMultimap.tpp"

#endif /* FSFW_CONTAINER_FIXEDORDEREDMULTIMAP_H_ */
