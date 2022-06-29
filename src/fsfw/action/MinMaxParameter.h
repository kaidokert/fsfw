#pragma once

#include "Parameter.h"

template <typename T>
class MinMaxParameter : public Parameter<T> {
#ifdef FSFW_INTROSPECTION
 private:
  MinMaxParameter(Action *owner, const char *name, T min, T max)
      : Parameter<T>(owner, name), min(min), max(max) {}

 public:
  static MinMaxParameter createMinMaxParameter(Action *owner, const char *name, T min, T max) {
    return MinMaxParameter(owner, name, min, max);
  }
  virtual double getMinFloating() override { return static_cast<double>(min); }
  virtual int64_t getMinSigned() override { return static_cast<int64_t>(min); }

  virtual double getMaxFloating() override { return static_cast<double>(max); }
  virtual int64_t getMaxSigned() override { return static_cast<int64_t>(max); }

#else
 private:
  MinMaxParameter(Action *owner, T min, T max) : Parameter<T>(owner), min(min), max(max) {}

 public:
  static MinMaxParameter createMinMaxParameter(Action *owner, T min, T max) {
    return MinMaxParameter(owner, min, max);
  }
#endif
 private:
  T min;
  T max;
};

#ifdef FSFW_INTROSPECTION
#define createMinMaxParameter(p1, p2, p3, p4) createMinMaxParameter(p1, p2, p3, p4)
#else
#define createMinMaxParameter(p1, p2, p3, p4) createMinMaxParameter(p1, p3, p4)
#endif