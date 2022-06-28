#pragma once

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerializeAdapter.h>

#include <boost/preprocessor.hpp>

// TODO ifdef EnumIF, consistent naming of functions arrays and macros (probably enum values and
// descriptions)

#include "EnumIF.h"
#include "EnumCommon.h"


#ifdef FSFW_INTROSPECTION

#define FSFW_ENUM(name, type, elements)                                                      \
  class name : public EnumIF, public SerializeIF {                                           \
   public:                                                                                   \
    enum : type { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(CLEAN_ENUM_ITEM, "", elements)) }; \
    name(type value) : value(value) {}                                                       \
    name() : value(-1) {}                                                                    \
    name(const name &other) : value(other.value) {}                                          \
    int64_t getValue() const override { return value; }                                      \
    operator type() { return value; }                                                        \
    name &operator=(name other) {                                                            \
      value = other.value;                                                                   \
      return *this;                                                                          \
    }                                                                                        \
    name &operator=(type value) {                                                            \
      this->value = value;                                                                   \
      return *this;                                                                          \
    }                                                                                        \
    CREATE_KEY_ARRAY(elements, type)                                                         \
    VALUE_CHECK(type)                                                                        \
    GET_INDEX()                                                                              \
    CREATE_DESCRIPTION_ARRAY(elements)                                                       \
    GET_DESCRIPTION_FUNC()                                                                   \
    virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,          \
                                    Endianness streamEndianness) const override {            \
      return SerializeAdapter::serialize<>(&value, buffer, size, maxSize, streamEndianness); \
    }                                                                                        \
    virtual size_t getSerializedSize() const override {                                      \
      return SerializeAdapter::getSerializedSize<>(&value);                                  \
    }                                                                                        \
    virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,                  \
                                      Endianness streamEndianness) override {                \
      return SerializeAdapter::deSerialize<>(&value, buffer, size, streamEndianness);        \
    }                                                                                        \
                                                                                             \
   private:                                                                                  \
    type value;                                                                              \
  };


#else

#define FSFW_ENUM(name, type, elements)                                     \
  enum class name : type {                                                  \
    BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(CLEAN_ENUM_ITEM, "", elements)) \
  };


#endif