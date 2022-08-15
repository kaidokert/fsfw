#ifndef FSFW_SERIALIZE_ENDIANCONVERTER_H_
#define FSFW_SERIALIZE_ENDIANCONVERTER_H_

#include <cstdint>
#include <cstring>

#include "fsfw/osal/Endiness.h"

/**
 * Helper class to convert variables or bitstreams between machine
 * endian and either big or little endian.
 * Machine endian is the endianness used by the machine running the
 * program and is one of big or little endian. As this is portable
 * code, it is not known at coding time which it is. At compile time
 * it is however, which is why this is implemented using compiler
 * macros and translates to a copy operation at runtime.
 *
 * This changes the layout of multi-byte variables in the machine's
 * memory. In most cases, you should not need to use this class.
 * Probably what you are looking for is the SerializeAdapter.
 * If you still decide you need this class, please read and understand
 * the code first.
 *
 * The order of the individual bytes of the multi-byte variable is
 * reversed, the byte at the highest address is moved to the lowest
 * address and vice versa, same for the bytes in between.
 *
 * Note that the conversion is also its inversion, that is converting
 * from machine to a specified endianness is the same operation as
 * converting from specified to machine (I looked it up, mathematicians
 * would call it an involution):
 *
 * X == convertBigEndian(convertBigEndian(X))
 *
 * Thus, there is only one function supplied to do the conversion.
 */
class EndianConverter {
 private:
  EndianConverter() = default;

 public:
  /**
   * Convert a typed variable between big endian and machine endian.
   * Intended for plain old datatypes.
   */
  template <typename T>
  static T convertBigEndian(T in) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    T tmp;
    auto *pointerOut = reinterpret_cast<uint8_t *>(&tmp);
    auto *pointerIn = reinterpret_cast<uint8_t *>(&in);
    for (size_t count = 0; count < sizeof(T); count++) {
      pointerOut[sizeof(T) - count - 1] = pointerIn[count];
    }
    return tmp;
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
    return in;
#else
#error Unknown Byte Order
#endif
  }

  /**
   * convert a bytestream representing a single variable between big endian
   * and machine endian.
   */
  static void convertBigEndian(uint8_t *out, const uint8_t *in, size_t size) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    for (size_t count = 0; count < size; count++) {
      out[size - count - 1] = in[count];
    }
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
    memcpy(out, in, size);
#endif
  }

  /**
   * Convert a typed variable between little endian and machine endian.
   * Intended for plain old datatypes.
   */
  template <typename T>
  static T convertLittleEndian(T in) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
    T tmp;
    auto *pointerOut = reinterpret_cast<uint8_t *>(&tmp);
    auto *pointerIn = reinterpret_cast<uint8_t *>(&in);
    for (size_t count = 0; count < sizeof(T); count++) {
      pointerOut[sizeof(T) - count - 1] = pointerIn[count];
    }
    return tmp;
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    return in;
#else
#error Unknown Byte Order
#endif
  }
  /**
   * convert a bytestream representing a single variable between little endian
   * and machine endian.
   */
  static void convertLittleEndian(uint8_t *out, const uint8_t *in, size_t size) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
    for (size_t count = 0; count < size; count++) {
      out[size - count - 1] = in[count];
    }
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    memcpy(out, in, size);
#endif
  }
};

#endif /* FSFW_SERIALIZE_ENDIANCONVERTER_H_ */
