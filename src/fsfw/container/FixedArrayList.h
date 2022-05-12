#ifndef FIXEDARRAYLIST_H_
#define FIXEDARRAYLIST_H_

#include <cmath>
#include <limits>

#include "ArrayList.h"
/**
 * \ingroup container
 */
template <typename T, size_t MAX_SIZE, typename count_t = uint8_t>
class FixedArrayList : public ArrayList<T, count_t> {
  static_assert(MAX_SIZE <= std::numeric_limits<count_t>::max(),
                "count_t is not large enough to hold MAX_SIZE");
 private:
  T data[MAX_SIZE];

 public:
  FixedArrayList() : ArrayList<T, count_t>(data, MAX_SIZE) {}

  FixedArrayList(const FixedArrayList& other) : ArrayList<T, count_t>(data, MAX_SIZE) {
    memcpy(this->data, other.data, sizeof(this->data));
    this->entries = data;
    this->size = other.size;
  }

  FixedArrayList& operator=(FixedArrayList other) {
    memcpy(this->data, other.data, sizeof(this->data));
    this->entries = data;
    this->size = other.size;
    return *this;
  }

  virtual ~FixedArrayList() {}
};

#endif /* FIXEDARRAYLIST_H_ */
