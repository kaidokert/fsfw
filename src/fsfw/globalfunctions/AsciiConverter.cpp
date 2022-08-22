#include "fsfw/globalfunctions/AsciiConverter.h"

#include <cmath>
#include <limits>

template <typename T>
ReturnValue_t AsciiConverter::scanAsciiDecimalNumber(const uint8_t** dataPtr, uint8_t len,
                                                     T* value) {
  if (len > std::numeric_limits<T>().digits10) {
    return TOO_LONG_FOR_TARGET_TYPE;
  }

  double temp;

  ReturnValue_t result = scanAsciiDecimalNumber_(dataPtr, len, &temp);

  *value = temp;

  return result;
}

ReturnValue_t AsciiConverter::scanAsciiHexByte(const uint8_t** dataPtr, uint8_t* value) {
  int8_t tmp;

  tmp = convertHexChar(*dataPtr);
  (*dataPtr)++;
  if (tmp == -1) {
    return INVALID_CHARACTERS;
  }
  if (tmp == -2) {
    tmp = 0;
  }

  *value = tmp << 4;

  tmp = convertHexChar(*dataPtr);
  (*dataPtr)++;
  if (tmp == -1) {
    return INVALID_CHARACTERS;
  }
  if (tmp != -2) {
    *value += tmp;
  } else {
    *value = *value >> 4;
  }

  return returnvalue::OK;
}

ReturnValue_t AsciiConverter::scanAsciiDecimalNumber_(uint8_t const** dataPtr, uint8_t len,
                                                      double* value) {
  uint8_t const* ptr = *dataPtr;
  int8_t sign = 1;
  float decimal = 0;
  bool abort = false;

  *value = 0;

  // ignore leading space
  ptr = clearSpace(ptr, len);

  while ((ptr - *dataPtr < len) && !abort) {
    switch (*ptr) {
      case '+':
        sign = 1;
        break;
      case '-':
        sign = -1;
        break;
      case '.':
        decimal = 1;
        break;
      case ' ':
      case 0x0d:
      case 0x0a:
        // ignore trailing space
        ptr = clearSpace(ptr, len - (ptr - *dataPtr)) -
              1;  // before aborting the loop, ptr will be incremented
        abort = true;
        break;
      default:
        if ((*ptr < 0x30) || (*ptr > 0x39)) {
          return INVALID_CHARACTERS;
        }
        *value = *value * 10 + (*ptr - 0x30);
        if (decimal > 0) {
          decimal *= 10;
        }
        break;
    }
    ptr++;
  }

  if (decimal == 0) {
    decimal = 1;
  }

  *value = *value / (decimal)*sign;

  *dataPtr = ptr;

  return returnvalue::OK;
}

ReturnValue_t AsciiConverter::printFloat(uint8_t* buffer, uint32_t bufferLength, float value,
                                         uint8_t decimalPlaces, uint32_t* printedSize) {
  *printedSize = 0;
  uint32_t streamposition = 0, integerSize;
  bool negative = (value < 0);
  int32_t digits = bufferLength - decimalPlaces - 1;
  if (digits <= 0) {
    return BUFFER_TOO_SMALL;
  }
  if (negative) {
    digits -= 1;
    buffer[streamposition++] = '-';
    value = -value;
  }
  float maximumNumber = pow(10, digits);
  if (value >= maximumNumber) {
    return BUFFER_TOO_SMALL;
  }
  // print the numbers before the decimal point;
  ReturnValue_t result =
      printInteger(buffer + streamposition, bufferLength - streamposition - decimalPlaces - 1,
                   value, &integerSize);
  if (result != returnvalue::OK) {
    return result;
  }
  streamposition += integerSize;
  // The decimal Point
  buffer[streamposition++] = '.';

  // Print the decimals
  uint32_t integerValue = value;
  value -= integerValue;
  value = value * pow(10, decimalPlaces);
  result = printInteger(buffer + streamposition, decimalPlaces, round(value), &integerSize, true);
  *printedSize = integerSize + streamposition;
  return result;
}

ReturnValue_t AsciiConverter::printInteger(uint8_t* buffer, uint32_t bufferLength, uint32_t value,
                                           uint32_t* printedSize, bool leadingZeros) {
  *printedSize = 0;
  if (bufferLength == 0) {
    return BUFFER_TOO_SMALL;
  }
  uint32_t maximumNumber = -1;
  if (bufferLength < 10) {
    maximumNumber = pow(10, bufferLength);
    if (value >= maximumNumber) {
      return BUFFER_TOO_SMALL;
    }
    maximumNumber /= 10;
  } else {
    if (!(value <= maximumNumber)) {
      return BUFFER_TOO_SMALL;
    }
    maximumNumber = 1000000000;
  }
  if (!leadingZeros && (value == 0)) {
    buffer[(*printedSize)++] = '0';
    return returnvalue::OK;
  }
  while (maximumNumber >= 1) {
    uint8_t number = value / maximumNumber;
    value = value - (number * maximumNumber);
    if (!leadingZeros && number == 0) {
      maximumNumber /= 10;
    } else {
      leadingZeros = true;
      buffer[(*printedSize)++] = '0' + number;
      maximumNumber /= 10;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t AsciiConverter::printSignedInteger(uint8_t* buffer, uint32_t bufferLength,
                                                 int32_t value, uint32_t* printedSize) {
  bool negative = false;
  if ((bufferLength > 0) && (value < 0)) {
    *buffer++ = '-';
    bufferLength--;
    value = -value;
    negative = true;
  }
  ReturnValue_t result = printInteger(buffer, bufferLength, value, printedSize);
  if (negative) {
    (*printedSize)++;
  }
  return result;
}

int8_t AsciiConverter::convertHexChar(const uint8_t* character) {
  if ((*character > 0x60) && (*character < 0x67)) {
    return *character - 0x61 + 10;
  } else if ((*character > 0x40) && (*character < 0x47)) {
    return *character - 0x41 + 10;
  } else if ((*character > 0x2F) && (*character < 0x3A)) {
    return *character - 0x30;
  } else if (*character == ' ') {
    return -2;
  }
  return -1;
}

template ReturnValue_t AsciiConverter::scanAsciiDecimalNumber<float>(const uint8_t** dataPtr,
                                                                     uint8_t len, float* value);
template ReturnValue_t AsciiConverter::scanAsciiDecimalNumber<uint8_t>(const uint8_t** dataPtr,
                                                                       uint8_t len, uint8_t* value);
template ReturnValue_t AsciiConverter::scanAsciiDecimalNumber<uint16_t>(const uint8_t** dataPtr,
                                                                        uint8_t len,
                                                                        uint16_t* value);
template ReturnValue_t AsciiConverter::scanAsciiDecimalNumber<double>(const uint8_t** dataPtr,
                                                                      uint8_t len, double* value);

const uint8_t* AsciiConverter::clearSpace(const uint8_t* data, uint8_t len) {
  while (len > 0) {
    if ((*data != ' ') && (*data != 0x0a) && (*data != 0x0d)) {
      return data;
    }
    data++;
    len--;
  }
  return data;
}
