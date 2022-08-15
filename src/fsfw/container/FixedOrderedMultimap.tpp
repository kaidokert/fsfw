#ifndef FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_TPP_
#define FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_TPP_

template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::insert(key_t key, T value,
                                                                         Iterator *storedValue) {
  if (_size == theMap.maxSize()) {
    return MAP_FULL;
  }
  size_t position = findNicePlace(key);
  memmove(static_cast<void *>(&theMap[position + 1]), static_cast<void *>(&theMap[position]),
          (_size - position) * sizeof(std::pair<key_t, T>));
  theMap[position].first = key;
  theMap[position].second = value;
  ++_size;
  if (storedValue != nullptr) {
    *storedValue = Iterator(&theMap[position]);
  }
  return returnvalue::OK;
}
template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::insert(std::pair<key_t, T> pair) {
  return insert(pair.first, pair.second);
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::exists(key_t key) const {
  ReturnValue_t result = KEY_DOES_NOT_EXIST;
  if (findFirstIndex(key) < _size) {
    result = returnvalue::OK;
  }
  return result;
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::erase(Iterator *iter) {
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
  return returnvalue::OK;
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::erase(key_t key) {
  size_t i;
  if ((i = findFirstIndex(key)) >= _size) {
    return KEY_DOES_NOT_EXIST;
  }
  do {
    removeFromPosition(i);
    i = findFirstIndex(key, i);
  } while (i < _size);
  return returnvalue::OK;
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline ReturnValue_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::find(key_t key, T **value) const {
  ReturnValue_t result = exists(key);
  if (result != returnvalue::OK) {
    return result;
  }
  *value = &theMap[findFirstIndex(key)].second;
  return returnvalue::OK;
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline size_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::findFirstIndex(key_t key,
                                                                          size_t startAt) const {
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

template <typename key_t, typename T, typename KEY_COMPARE>
inline size_t FixedOrderedMultimap<key_t, T, KEY_COMPARE>::findNicePlace(key_t key) const {
  size_t i = 0;
  for (i = 0; i < _size; ++i) {
    if (myComp(key, theMap[i].first)) {
      return i;
    }
  }
  return i;
}

template <typename key_t, typename T, typename KEY_COMPARE>
inline void FixedOrderedMultimap<key_t, T, KEY_COMPARE>::removeFromPosition(size_t position) {
  if (_size <= position) {
    return;
  }
  memmove(static_cast<void *>(&theMap[position]), static_cast<void *>(&theMap[position + 1]),
          (_size - position - 1) * sizeof(std::pair<key_t, T>));
  --_size;
}

#endif /* FRAMEWORK_CONTAINER_FIXEDORDEREDMULTIMAP_TPP_ */
