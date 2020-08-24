#ifndef SERIALIZEIF_H_
#define SERIALIZEIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include <stddef.h>

/**
 * \defgroup serialize Serialization
 * Contains serialisation services.
 */

/**
 * Translation of objects into data streams.
 * \ingroup serialize
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
