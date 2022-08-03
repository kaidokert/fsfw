#include "VarLenField.h"

#include "fsfw/FSFW.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serviceinterface.h"

cfdp::VarLenField::VarLenField(cfdp::WidthInBytes width, size_t value) : VarLenField() {
  ReturnValue_t result = this->setValue(width, value);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_DISABLE_PRINTOUT == 0
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "cfdp::VarLenField: Setting value failed" << std::endl;
#else
    sif::printWarning("cfdp::VarLenField: Setting value failed\n");
#endif
#endif
  }
}

cfdp::VarLenField::VarLenField() : width(cfdp::WidthInBytes::ONE_BYTE) { value.oneByte = 0; }

cfdp::WidthInBytes cfdp::VarLenField::getWidth() const { return width; }

ReturnValue_t cfdp::VarLenField::setValue(cfdp::WidthInBytes widthInBytes, size_t value_) {
  switch (widthInBytes) {
    case (cfdp::WidthInBytes::ONE_BYTE): {
      if (value_ > UINT8_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.oneByte = value_;
      break;
    }
    case (cfdp::WidthInBytes::TWO_BYTES): {
      if (value_ > UINT16_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.twoBytes = value_;
      break;
    }
    case (cfdp::WidthInBytes::FOUR_BYTES): {
      if (value_ > UINT32_MAX) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }
      this->value.fourBytes = value_;
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

ReturnValue_t cfdp::VarLenField::deSerialize(cfdp::WidthInBytes width_, const uint8_t **buffer,
                                             size_t *size, Endianness streamEndianness) {
  this->width = width_;
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
