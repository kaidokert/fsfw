#ifndef FSFW_GLOBALFUNCTIONS_TYPE_H_
#define FSFW_GLOBALFUNCTIONS_TYPE_H_

#include <type_traits>

#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serialize/SerializeIF.h"

/**
 * @brief 	Type definition for CCSDS or ECSS.
 */
class Type : public SerializeIF {
 public:
  enum ActualType_t {
    UINT8_T,
    INT8_T,
    UINT16_T,
    INT16_T,
    UINT32_T,
    INT32_T,
    FLOAT,
    DOUBLE,
    UNKNOWN_TYPE
  };

  Type();

  Type(ActualType_t actualType);

  Type(const Type &type);

  Type &operator=(Type rhs);

  Type &operator=(ActualType_t actualType);

  operator ActualType_t() const;

  bool operator==(const Type &rhs);
  bool operator!=(const Type &rhs);

  uint8_t getSize() const;

  ReturnValue_t getPtcPfc(uint8_t *ptc, uint8_t *pfc) const;

  static ActualType_t getActualType(uint8_t ptc, uint8_t pfc);

  virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                  Endianness streamEndianness) const override;

  virtual size_t getSerializedSize() const override;

  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) override;

 private:
  ActualType_t actualType;
};

template <typename T>
struct PodTypeConversion {
  static_assert(not std::is_same<T, bool>::value,
                "Do not use boolean for the PoolEntry type, use uint8_t "
                "instead! The ECSS standard defines a boolean as a one bit "
                "field. Therefore it is preferred to store a boolean as an "
                "uint8_t");
  static const Type::ActualType_t type = Type::UNKNOWN_TYPE;
};
template <>
struct PodTypeConversion<uint8_t> {
  static const Type::ActualType_t type = Type::UINT8_T;
};
template <>
struct PodTypeConversion<uint16_t> {
  static const Type::ActualType_t type = Type::UINT16_T;
};
template <>
struct PodTypeConversion<uint32_t> {
  static const Type::ActualType_t type = Type::UINT32_T;
};
template <>
struct PodTypeConversion<int8_t> {
  static const Type::ActualType_t type = Type::INT8_T;
};
template <>
struct PodTypeConversion<int16_t> {
  static const Type::ActualType_t type = Type::INT16_T;
};
template <>
struct PodTypeConversion<int32_t> {
  static const Type::ActualType_t type = Type::INT32_T;
};
template <>
struct PodTypeConversion<float> {
  static const Type::ActualType_t type = Type::FLOAT;
};
template <>
struct PodTypeConversion<double> {
  static const Type::ActualType_t type = Type::DOUBLE;
};

#endif /* FSFW_GLOBALFUNCTIONS_TYPE_H_ */
