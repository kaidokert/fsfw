#define CLEAN_ENUM_ITEM(r, data, element)                                          \
  BOOST_PP_IF(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(element), 2),                       \
              (BOOST_PP_TUPLE_ELEM(0, element) = BOOST_PP_TUPLE_ELEM(1, element)), \
              (BOOST_PP_TUPLE_ELEM(0, element)))

#if defined FSFW_ENUM_VALUE_CHECKS || defined FSFW_INTROSPECTION


#define GET_KEY(r, data, element) (BOOST_PP_TUPLE_ELEM(0, element))
#define GET_DESCRIPTION(r, data, element)                                                       \
  BOOST_PP_IF(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(element), 2), (BOOST_PP_TUPLE_ELEM(2, element)), \
              (BOOST_PP_TUPLE_ELEM(1, element)))

#define CREATE_KEY_ARRAY(enum_elements, type)                                                \
  /*was static constexpr, but clang won't compile that*/                                     \
  int64_t elements[BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_FOR_EACH(GET_KEY, "", enum_elements))] = { \
      BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(GET_KEY, "", enum_elements))};                 \
  const int64_t *getElements() const override { return elements; }                           \
  size_t getSize() const override {                                                          \
    return BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_FOR_EACH(GET_KEY, "", enum_elements));             \
  }
#define VALUE_CHECK(type)                                                 \
  bool isValid() const override {                                         \
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++) { \
      if (value == elements[i]) {                                         \
        return true;                                                      \
      }                                                                   \
    }                                                                     \
    return false;                                                         \
  }

#ifdef FSFW_INTROSPECTION
#define CREATE_DESCRIPTION_ARRAY(elements)                                                       \
  /*was static constexpr, but clang won't compile that*/                                         \
  const char                                                                                     \
      *descriptions[BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_FOR_EACH(GET_DESCRIPTION, "", elements))] = { \
          BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(GET_DESCRIPTION, "", elements))};              \
  const char *const *getDescriptions() const override { return descriptions; }
#define GET_INDEX()                                                       \
  size_t getIndex(int64_t value) const override {                         \
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++) { \
      if (value == elements[i]) {                                         \
        return i;                                                         \
      }                                                                   \
    }                                                                     \
    return -1;                                                            \
  }
#define GET_DESCRIPTION_FUNC()                        \
  const char *getDescription() const override {       \
    if (getIndex(value) == static_cast<size_t>(-1)) { \
      return nullptr;                                      \
    } else {                                          \
      return descriptions[getIndex(value)];           \
    }                                                 \
  }
#else
#define GET_INDEX()
#define CREATE_DESCRIPTION_ARRAY(elements)
#define GET_DESCRIPTION_FUNC()
#endif

#endif