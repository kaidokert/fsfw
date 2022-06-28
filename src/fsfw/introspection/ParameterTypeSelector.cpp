#include "ParameterTypeSelector.h"

#include <stdint.h>

#include "Types.h"

#ifdef FSFW_INTROSPECTION

template <typename T>
Types::ParameterType ParameterTypeSelector::getType() {
    return Types::UNSUPPORTED;
  }

template <>
Types::ParameterType ParameterTypeSelector::getType<uint8_t>() {
  return Types::SIGNED;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<int8_t>() {
  return Types::SIGNED;
}

template <>
Types::ParameterType ParameterTypeSelector::getType<uint16_t>() {
  return Types::SIGNED;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<int16_t>() {
  return Types::SIGNED;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<uint32_t>() {
  return Types::SIGNED;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<int32_t>() {
  return Types::SIGNED;
}
// template <>
// Types::ParameterType ParameterTypeSelector::getType<uint64_t>() {
//   return Types::UNSIGNED;
// }
template <>
Types::ParameterType ParameterTypeSelector::getType<int64_t>() {
  return Types::SIGNED;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<float>() {
  return Types::FLOATING;
}
template <>
Types::ParameterType ParameterTypeSelector::getType<double>() {
  return Types::FLOATING;
}

#endif