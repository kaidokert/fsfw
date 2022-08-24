#pragma once

#include <stdio.h>

#include "Action.h"
#include <fsfw/introspection/Types.h>
#include <fsfw/introspection/TypesHelper.h>
#include "ParameterIF.h"
// TODO: ifdef introspection stuff



template <typename T>
class Parameter : public ParameterIF {
 protected:
#ifdef FSFW_INTROSPECTION
  Parameter(Action *owner, const char *name)
      : name(name)
#else
  Parameter(Action *owner)
#endif
  {
    owner->registerParameter(this);
  }

 public:
#ifdef FSFW_INTROSPECTION
  static Parameter createParameter(Action *owner, const char *name) {
    return Parameter(owner, name);
  }
#else
  static Parameter createParameter(Action *owner) { return Parameter(owner); }
#endif

  bool isValid() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::isValid(&value);
  }

  operator T(){
    return value;
  }

  Parameter& operator =(const T& newValue){
    value = newValue;
    return *this;
  }

#ifdef FSFW_INTROSPECTION
  Types::ParameterType getType() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::template getType<T>();
  }
#endif

  T value;

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&value, buffer, size, maxSize, streamEndianness);
  }

  size_t getSerializedSize() const override { return SerializeAdapter::getSerializedSize(&value); }

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
  }

#ifdef FSFW_INTROSPECTION
  double getFloating() override { return (double)value; }
  int64_t getSigned() override { return (int64_t)value; }

  bool setFloating(double value) override {
    if (getType() != Types::FLOATING) {
      return false;
    }
    this->value = T(value);
    return true;
  }

  bool setSigned(int64_t value) override {
    if ((getType() != Types::SIGNED) && (getType() != Types::ENUM)) {
      return false;
    }
    this->value = T(value);
    return true;
  }

  double getMinFloating() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::template getMin<T>();
  }
  int64_t getMinSigned() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::template getMin<T>();
  }

  double getMaxFloating() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::template getMax<T>();
  }
  int64_t getMaxSigned() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::template getMax<T>();
  }

  std::vector<int64_t> getEnumValues() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::getEnumValues(&value);
  }
  const char *const *getEnumDescriptions() override {
    return enumHelper<std::is_base_of<EnumIF, T>::value>::getEnumDescriptions(&value);
  }

  const char *getName() override { return name; }

 private:
  const char *name;
#endif
};

#ifdef FSFW_INTROSPECTION
#define createParameter(p1, p2) createParameter(p1, p2)
#else
#define createParameter(p1, p2) createParameter(p1)
#endif