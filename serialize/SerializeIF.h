/*
 * SerializeIF.h
 *
 *  Created on: 19.03.2014
 *      Author: baetz
 */

#ifndef SERIALIZEIF_H_
#define SERIALIZEIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

class SerializeIF {
public:
	static const uint8_t INTERFACE_ID = SERIALIZE_IF;
	static const ReturnValue_t BUFFER_TOO_SHORT = MAKE_RETURN_CODE(1);
	static const ReturnValue_t STREAM_TOO_SHORT = MAKE_RETURN_CODE(2);
	static const ReturnValue_t TOO_MANY_ELEMENTS = MAKE_RETURN_CODE(3);

	virtual ~SerializeIF() {
	}

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const = 0;

	virtual uint32_t getSerializedSize() const = 0;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) = 0;

};

#endif /* SERIALIZEIF_H_ */
