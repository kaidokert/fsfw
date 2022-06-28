#pragma once

#include <limits>
#include <vector>

#include "Enum.h"
#include "Types.h"
#include "ParameterTypeSelector.h"

template <bool>
class enumHelper;

template <>
class enumHelper<true> {
 public:
  static bool isValid(EnumIF *anEnum) { return anEnum->isValid(); }

#ifdef FSFW_INTROSPECTION
  template <typename T>
  static Types::ParameterType getType() {
    return Types::ENUM;
  }

  template <typename T>
  static T getMin() {
    return 0;
  }

  template <typename T>
  static T getMax() {
    return 0;
  }

  static std::vector<int64_t> getEnumValues() { return std::vector<int64_t>(); }

  static std::vector<int64_t> getEnumValues(EnumIF *anEnum) {
    std::vector<int64_t> vector;
    for (size_t i = 0; i < anEnum->getSize(); i++) {
      vector.push_back(anEnum->getElements()[i]);
    }
    return vector;
  }

  static const char *const *getEnumDescriptions(EnumIF *anEnum) {
    return anEnum->getDescriptions();
  }
#endif
};

template <>
class enumHelper<false> {
 public:
  static bool isValid(void *) { return true; }

#ifdef FSFW_INTROSPECTION
  template <typename T>
  static Types::ParameterType getType() {
    return ParameterTypeSelector::getType<T>();
  }

  template <typename T>
  static T getMin() {
    return std::numeric_limits<T>::lowest();
  }
  template <typename T>
  static T getMax() {
    return std::numeric_limits<T>::max();
  }

  static std::vector<int64_t> getEnumValues(void *) { return std::vector<int64_t>(); }

  static const char *const *getEnumDescriptions(void *) { return nullptr; }
#endif
};