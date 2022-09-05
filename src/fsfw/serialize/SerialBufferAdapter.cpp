#include "fsfw/serialize/SerialBufferAdapter.h"

#include <cstring>

#include "fsfw/serviceinterface/ServiceInterface.h"

template <typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(const uint8_t* buffer, count_t bufferLength,
                                                  bool serializeLength)
    : serializeLength(serializeLength),
      constBuffer(buffer),
      buffer(nullptr),
      bufferLength(bufferLength) {}

template <typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(uint8_t* buffer, count_t bufferLength,
                                                  bool serializeLength)
    : serializeLength(serializeLength),
      constBuffer(buffer),
      buffer(buffer),
      bufferLength(bufferLength) {}

template <typename count_t>
SerialBufferAdapter<count_t>::~SerialBufferAdapter() = default;

template <typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::serialize(uint8_t** buffer_, size_t* size,
                                                      size_t maxSize,
                                                      Endianness streamEndianness) const {
  if (serializeLength) {
    ReturnValue_t result =
        SerializeAdapter::serialize(&bufferLength, buffer_, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }

  if (*size + bufferLength > maxSize) {
    return BUFFER_TOO_SHORT;
  }

  if (this->constBuffer != nullptr) {
    std::memcpy(*buffer_, this->constBuffer, bufferLength);
  } else if (this->buffer != nullptr) {
    // This will propably be never reached, constBuffer should always be
    // set if non-const buffer is set.
    std::memcpy(*buffer_, this->buffer, bufferLength);
  } else {
    return returnvalue::FAILED;
  }
  *size += bufferLength;
  (*buffer_) += bufferLength;
  return returnvalue::OK;
}

template <typename count_t>
size_t SerialBufferAdapter<count_t>::getSerializedSize() const {
  if (serializeLength) {
    return bufferLength + SerializeAdapter::getSerializedSize(&bufferLength);
  } else {
    return bufferLength;
  }
}

template <typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::deSerialize(const uint8_t** buffer_, size_t* size,
                                                        Endianness streamEndianness) {
  if (this->buffer == nullptr) {
    return returnvalue::FAILED;
  }

  if (serializeLength) {
    count_t lengthField = 0;
    ReturnValue_t result =
        SerializeAdapter::deSerialize(&lengthField, buffer_, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (lengthField > bufferLength) {
      return TOO_MANY_ELEMENTS;
    }
    bufferLength = lengthField;
  }

  if (bufferLength <= *size) {
    *size -= bufferLength;
    std::memcpy(this->buffer, *buffer_, bufferLength);
    (*buffer_) += bufferLength;
    return returnvalue::OK;
  } else {
    return STREAM_TOO_SHORT;
  }
}

template <typename count_t>
uint8_t* SerialBufferAdapter<count_t>::getBuffer() {
  if (buffer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Wrong access function for stored type !"
                  " Use getConstBuffer()."
               << std::endl;
#endif
    return nullptr;
  }
  return buffer;
}

template <typename count_t>
const uint8_t* SerialBufferAdapter<count_t>::getConstBuffer() const {
  if (constBuffer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "SerialBufferAdapter::getConstBuffer:"
                  " Buffers are unitialized!"
               << std::endl;
#endif
    return nullptr;
  }
  return constBuffer;
}

template <typename count_t>
void SerialBufferAdapter<count_t>::setConstBuffer(const uint8_t* buf, count_t bufLen) {
  this->buffer = nullptr;
  this->bufferLength = bufLen;
  this->constBuffer = buf;
}

// forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;
template class SerialBufferAdapter<uint64_t>;
