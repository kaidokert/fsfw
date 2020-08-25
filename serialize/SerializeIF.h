#ifndef FSFW_SERIALIZE_SERIALIZEIF_H_
#define FSFW_SERIALIZE_SERIALIZEIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include <stddef.h>

/**
 * \defgroup serialize Serialization
 * Contains serialization services.
 */

/**
 * Translation of objects into data streams and from data streams.
 * \ingroup serialize
 */
class SerializeIF {
public:
	enum class Endianness : uint8_t {
		BIG, LITTLE, MACHINE
	};

	static const uint8_t INTERFACE_ID = CLASS_ID::SERIALIZE_IF;
	static const ReturnValue_t BUFFER_TOO_SHORT = MAKE_RETURN_CODE(1); // !< The given buffer in serialize is too short
	static const ReturnValue_t STREAM_TOO_SHORT = MAKE_RETURN_CODE(2); // !< The input stream in deserialize is too short
	static const ReturnValue_t TOO_MANY_ELEMENTS = MAKE_RETURN_CODE(3);// !< There are too many elements to be deserialized

	virtual ~SerializeIF() {
	}
	/**
	 * Function to serialize the object into a buffer with maxSize at the current size.
	 *
	 * Implementations of this function must increase the size variable and move the buffer pointer.
	 *
	 *
	 * Custom implementations might use additional return values.
	 *
	 * @param buffer Buffer to serialize into, will be set to the current write location
	 * @param size Size that has been used in the buffer already, will be increase by the function
	 * @param maxSize Max size of the buffer.
	 * @param streamEndianness Endianness of the serialized data according to SerializeIF::Endianness
	 * @return
	 * 		- @¢ BUFFER_TOO_SHORT The given buffer in is too short
	 * 		- @c RETURN_FAILED Generic error
	 * 		- @c RETURN_OK Successful serialization
	 */
	virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
			size_t maxSize, Endianness streamEndianness) const = 0;

	/**
	 * Gets the size of a object if it would be serialized in a buffer
	 * @return Size of serialized object
	 */
	virtual size_t getSerializedSize() const = 0;

	/**
	 * Deserializes a object from a given buffer of given size.
	 *
	 * Buffer must be moved to the current read location by the implementation
	 * of this function. Size must be decreased by the implementation.
	 *
	 * Custom implementations might use additional return values.
	 *
	 * @param buffer Buffer to deSerialize from. Will be moved by the function.
	 * @param size Remaining size of the buffer to read from. Will be decreased by function.
	 * @param streamEndianness Endianness as in according to SerializeIF::Endianness
	 * @return
	 * 	- @c STREAM_TOO_SHORT The input stream is too short to deSerialize the object
	 * 	- @c TOO_MANY_ELEMENTS The buffer has more inputs than expected
	 * 	- @c RETURN_FAILED Generic Error
	 * 	- @c RETURN_OK Successful deserialization
	 */
	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) = 0;

};

#endif /* FSFW_SERIALIZE_SERIALIZEIF_H_ */
