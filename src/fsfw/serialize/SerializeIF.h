#ifndef FSFW_SERIALIZE_SERIALIZEIF_H_
#define FSFW_SERIALIZE_SERIALIZEIF_H_

#include <cstddef>

#include "fsfw/returnvalues/returnvalue.h"

/**
 * @defgroup serialize Serialization
 * Contains serialization services.
 */

/**
 * @brief 	Translation of objects into data streams and from data streams.
 * @details
 * Also provides options to convert from/to data with different endianness.
 * variables.
 * @ingroup serialize
 */
class SerializeIF {
 public:
  enum class Endianness : uint8_t {
    BIG,
    LITTLE,
    MACHINE,
    NETWORK = BIG  // Added for convenience like htons on sockets
  };

  static const uint8_t INTERFACE_ID = CLASS_ID::SERIALIZE_IF;
  static const ReturnValue_t BUFFER_TOO_SHORT =
      MAKE_RETURN_CODE(1);  // !< The given buffer in serialize is too short
  static const ReturnValue_t STREAM_TOO_SHORT =
      MAKE_RETURN_CODE(2);  // !< The input stream in deserialize is too short
  static const ReturnValue_t TOO_MANY_ELEMENTS =
      MAKE_RETURN_CODE(3);  // !< There are too many elements to be deserialized

  virtual ~SerializeIF() = default;
  /**
   * @brief
   * Function to serialize the object into a buffer with maxSize. Size represents the written
   * amount. If a part of the buffer has been used already, size must be set to the used amount of
   * bytes.
   *
   * @details
   * Implementations of this function must increase the size variable and move the buffer pointer.
   * MaxSize must be checked by implementations of this function
   * and BUFFER_TOO_SHORT has to be returned if size would be larger than maxSize.
   *
   * Custom implementations might use additional return values.
   *
   * @param[in/out] buffer Buffer to serialize into, will be set to the current write location
   * @param[in/out] size Size that has been used in the buffer already, will be increased by the
   * function
   * @param[in] maxSize The size of the buffer that is allowed to be used for serialize.
   * @param[in] streamEndianness Endianness of the serialized data according to
   * SerializeIF::Endianness
   * @return
   * 		- @c BUFFER_TOO_SHORT The given buffer in is too short
   * 		- @c returnvalue::FAILED Generic error
   * 		- @c returnvalue::OK Successful serialization
   */
  [[nodiscard]] virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                                Endianness streamEndianness) const = 0;
  /**
   * Forwards to regular @serialize call with big (network) endianness
   */
  [[nodiscard]] virtual ReturnValue_t serializeBe(uint8_t **buffer, size_t *size,
                                                  size_t maxSize) const {
    return serialize(buffer, size, maxSize, SerializeIF::Endianness::NETWORK);
  }

  /**
   * Gets the size of a object if it would be serialized in a buffer
   * @return Size of serialized object
   */
  [[nodiscard]] virtual size_t getSerializedSize() const = 0;

  /**
   * @brief
   * Deserializes a object from a given buffer of given size.
   *
   * @details
   * Buffer must be moved to the current read location by the implementation
   * of this function. Size must be decreased by the implementation.
   * Implementations are not allowed to alter the buffer as indicated by const pointer.
   *
   * Custom implementations might use additional return values.
   *
   * @param[in/out] buffer Buffer to deSerialize from. Will be moved by the function.
   * @param[in/out] size Remaining size of the buffer to read from. Will be decreased by function.
   * @param[in] streamEndianness Endianness as in according to SerializeIF::Endianness
   * @return
   * 	- @c STREAM_TOO_SHORT The input stream is too short to deSerialize the object
   * 	- @c TOO_MANY_ELEMENTS The buffer has more inputs than expected
   * 	- @c returnvalue::FAILED Generic Error
   * 	- @c returnvalue::OK Successful deserialization
   */
  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) = 0;
  /**
   * Forwards to regular @deSerialize call with big (network) endianness
   */
  virtual ReturnValue_t deSerializeBe(const uint8_t **buffer, size_t *size) {
    return deSerialize(buffer, size, SerializeIF::Endianness::NETWORK);
  }

  /**
   * Helper method which can be used if serialization should be performed without any additional
   * pointer arithmetic on a passed buffer pointer
   * @param buffer
   * @param maxSize
   * @param streamEndianness
   * @return
   */
  [[nodiscard]] virtual ReturnValue_t serialize(uint8_t *buffer, size_t &serLen, size_t maxSize,
                                                Endianness streamEndianness) const {
    size_t tmpSize = 0;
    ReturnValue_t result = serialize(&buffer, &tmpSize, maxSize, streamEndianness);
    serLen = tmpSize;
    return result;
  }
  /**
   * Forwards to regular @serialize call with big (network) endianness
   */
  [[nodiscard]] virtual ReturnValue_t serializeBe(uint8_t *buffer, size_t &serLen,
                                                  size_t maxSize) const {
    return serialize(buffer, serLen, maxSize, SerializeIF::Endianness::NETWORK);
  }

  /**
   * Helper methods which can be used if deserialization should be performed without any additional
   * pointer arithmetic on a passed buffer pointer
   * @param buffer
   * @param maxSize
   * @param streamEndianness
   * @return
   */
  virtual ReturnValue_t deSerialize(const uint8_t *buffer, size_t &deserSize, size_t maxSize,
                                    Endianness streamEndianness) {
    size_t deserLen = maxSize;
    ReturnValue_t result = deSerialize(&buffer, &deserLen, streamEndianness);
    deserSize = maxSize - deserLen;
    return result;
  }
  /**
   * Forwards to regular @serialize call with big (network) endianness
   */
  virtual ReturnValue_t deSerializeBe(const uint8_t *buffer, size_t &deserSize, size_t maxSize) {
    return deSerialize(buffer, deserSize, maxSize, SerializeIF::Endianness::NETWORK);
  }
};

#endif /* FSFW_SERIALIZE_SERIALIZEIF_H_ */
