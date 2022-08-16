#ifndef ASCIICONVERTER_H_
#define ASCIICONVERTER_H_

#include "../returnvalues/returnvalue.h"

class AsciiConverter {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::ASCII_CONVERTER;
  static const ReturnValue_t TOO_LONG_FOR_TARGET_TYPE = MAKE_RETURN_CODE(1);
  static const ReturnValue_t INVALID_CHARACTERS = MAKE_RETURN_CODE(2);
  static const ReturnValue_t BUFFER_TOO_SMALL = MAKE_RETURN_CODE(0x3);

  template <typename T>
  static ReturnValue_t scanAsciiDecimalNumber(const uint8_t **dataPtr, uint8_t len, T *value);

  static ReturnValue_t scanAsciiHexByte(const uint8_t **dataPtr, uint8_t *value);

  static ReturnValue_t printFloat(uint8_t *buffer, uint32_t bufferLength, float value,
                                  uint8_t decimalPlaces, uint32_t *printedSize);

  static ReturnValue_t printInteger(uint8_t *buffer, uint32_t bufferLength, uint32_t value,
                                    uint32_t *printedSize, bool leadingZeros = false);

  static ReturnValue_t printSignedInteger(uint8_t *buffer, uint32_t bufferLength, int32_t value,
                                          uint32_t *printedSize);

 private:
  AsciiConverter();
  static ReturnValue_t scanAsciiDecimalNumber_(const uint8_t **dataPtr, uint8_t len, double *value);

  static int8_t convertHexChar(const uint8_t *character);

  static const uint8_t *clearSpace(const uint8_t *data, uint8_t len);
};

#endif /* ASCIICONVERTER_H_ */
