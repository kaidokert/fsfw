#pragma once

#include "Types.h"

#ifdef FSFW_INTROSPECTION

class ParameterTypeSelector {
 public:
  template <typename T>
  static Types::ParameterType getType();
};

#endif