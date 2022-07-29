#ifndef FSFW_UTIL_OBJECTID_H
#define FSFW_UTIL_OBJECTID_H

#include "fsfw/objectmanager.h"
#include "UnsignedByteField.h"

#include <functional>

class ObjectId: public UnsignedByteField<object_id_t> {
 public:
  ObjectId(object_id_t id, const char* name): UnsignedByteField<object_id_t>(id), name_(name) {}

  [[nodiscard]] const char* name() const {
    return name_;
  }

  [[nodiscard]] object_id_t id() const {
    return getValue();
  }

  bool operator==(const ObjectId& other) const {
    return id() == other.id();
  }

  bool operator!=(const ObjectId& other) const {
    return id() != other.id();
  }

  bool operator<(const ObjectId& other) const {
    return id() < other.id();
  }

  bool operator>(const ObjectId& other) const {
    return id() > other.id();
  }

  bool operator>=(const ObjectId& other) const {
    return id() >= other.id();
  }

  bool operator<=(const ObjectId& other) const {
    return id() <= other.id();
  }
 private:
  const char* name_;
};

template<>
struct std::hash<ObjectId>
{
  std::size_t operator()(ObjectId const& s) const noexcept
  {
    return std::hash<object_id_t>{}(s.id());
  }
};

#endif  // FSFW_UTIL_OBJECTID_H
