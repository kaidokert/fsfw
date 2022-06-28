#pragma once

#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#include <boost/preprocessor.hpp>

// TODO ifdef EnumIF, consistent naming of functions arrays and macros (probably enum values and
// descriptions)

#include "EnumIF.h"
#include "EnumCommon.h"

#ifdef FSFW_INTROSPECTION

#define FSFW_CLASSLESS_ENUM(name, type, elements)                                            \
  enum : type { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(CLEAN_ENUM_ITEM, "", elements)) };   \
                                                                                             \
  class name : public EnumIF {                                                               \
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
   private:                                                                                  \
    type value;                                                                              \
  };

#else

#define FSFW_CLASSLESS_ENUM(name, type, elements) \
  enum name : type { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(CLEAN_ENUM_ITEM, "", elements)) };

#endif