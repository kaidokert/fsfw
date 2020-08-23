#ifndef SERIALIZEIF_H_
#define SERIALIZEIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include <cstddef>

/**
 * @defgroup serialize Serialization
 * Contains serialisation services.
 */

/**
 * @brief An interface for alle classes which require
 *        translation of objects data into data streams and vice-versa.
 * @details
 * If the target architecture is little endian (e.g. ARM), any data types
 * created might  have the wrong endianess if they are to be used for the FSFW.
 * Depending on the system architecture, endian correctness must be assured,
 * This is important for incoming and outgoing data. The internal handling
 * of data should be performed in the native system endianness.
 * There are three ways to copy data (with different options to ensure
 * endian correctness):
 *
 *   1. Use the @c AutoSerializeAdapter::deSerialize function (with
 *      the endian flag)
 *   2. Perform a bitshift operation (with correct order)
 *   3. @c memcpy (with @c EndianSwapper if necessary)
 *
 * When serializing for downlink, the packets are generally serialized
 * assuming big endian data format like seen in TmPacketStored.cpp for example.
 *
 * @ingroup serialize
 */
class SerializeIF {
public:
	enum class Endianness : uint8_t {
		BIG, LITTLE, MACHINE
	};

	static const uint8_t INTERFACE_ID = CLASS_ID::SERIALIZE_IF;
	static const ReturnValue_t BUFFER_TOO_SHORT = MAKE_RETURN_CODE(1);
	static const ReturnValue_t STREAM_TOO_SHORT = MAKE_RETURN_CODE(2);
	static const ReturnValue_t TOO_MANY_ELEMENTS = MAKE_RETURN_CODE(3);

	virtual ~SerializeIF() {
	}

	virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
			size_t maxSize, Endianness streamEndianness) const = 0;

	virtual size_t getSerializedSize() const = 0;

	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) = 0;

};

#endif /* SERIALIZEIF_H_ */
