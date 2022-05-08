#include "VarLenField.h"

#include "fsfw/FSFW.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serviceinterface.h"

cfdp::VarLenField::VarLenField(cfdp::WidthInBytes width, size_t value) : VarLenField() {
  ReturnValue_t result = this->setValue(width, value);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    FSFW_FLOGW("{}", "cfdp::VarLenField: Setting value failed\n");
  }
}

cfdp::VarLenField::VarLenField() : width(cfdp::WidthInBytes::ONE_BYTE) { value.oneByte = 0; }

cfdp::WidthInBytes cfdp::VarLenField::getWidth() const { return width; }

ReturnValue_t cfdp::VarLenField::setValue(cfdp::WidthInBytes widthInBytes, size_t value) {
  switch (widthInBytes) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      if (value > UINT8_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.oneByte = value;
      break;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      if (value > UINT16_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.twoBytes = value;
      break;
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      if (value > UINT32_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.fourBytes = value;
      break;
    }
    default: {
      break;
    }
  }
  this->width = widthInBytes;
  return HasReturnvaluesIF::RETURN_OK;
}

size_t cfdp::VarLenField::getValue() const {
  switch (width) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      return value.oneByte;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      return value.twoBytes;
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      return value.fourBytes;
    }
  }
  return 0;
}

ReturnValue_t cfdp::VarLenField::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                           Endianness streamEndianness) const {
  switch (width) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      if (*size + 1 > maxSize) {
        return BUFFER_TOO_SHORT;
      }
      **buffer = value.oneByte;
      *size += 1;
      *buffer += 1;
      return HasReturnvaluesIF::RETURN_OK;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      return SerializeAdapter::serialize(&value.twoBytes, buffer, size, maxSize, streamEndianness);
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      return SerializeAdapter::serialize(&value.fourBytes, buffer, size, maxSize, streamEndianness);
    }
    default: {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
}

size_t cfdp::VarLenField::getSerializedSize() const { return width; }

ReturnValue_t cfdp::VarLenField::deSerialize(cfdp::WidthInBytes width, const uint8_t **buffer,
                                             size_t *size, Endianness streamEndianness) {
  this->width = width;
  return deSerialize(buffer, size, streamEndianness);
}

ReturnValue_t cfdp::VarLenField::deSerialize(const uint8_t **buffer, size_t *size,
                                             Endianness streamEndianness) {
  switch (width) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      value.oneByte = **buffer;
      *size += 1;
      return HasReturnvaluesIF::RETURN_OK;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      return SerializeAdapter::deSerialize(&value.twoBytes, buffer, size, streamEndianness);
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      return SerializeAdapter::deSerialize(&value.fourBytes, buffer, size, streamEndianness);
    }
    default: {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }
}
