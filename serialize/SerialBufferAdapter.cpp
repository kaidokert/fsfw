#include <framework/serialize/SerialBufferAdapter.h>
#include <cstring>

SerialBufferAdapter::SerialBufferAdapter(const uint8_t* buffer,
		uint32_t bufferLength) :
		constBuffer(buffer), buffer(NULL), bufferLength(bufferLength) {
}

SerialBufferAdapter::SerialBufferAdapter(uint8_t* buffer, uint32_t bufferLength) :
		constBuffer(NULL), buffer(buffer), bufferLength(bufferLength) {
}

SerialBufferAdapter::~SerialBufferAdapter() {
}

ReturnValue_t SerialBufferAdapter::serialize(uint8_t** buffer, uint32_t* size,
		const uint32_t max_size, bool bigEndian) const {
	if (*size + bufferLength > max_size) {
		return BUFFER_TOO_SHORT;
	} else {
		if (this->constBuffer != NULL) {
			memcpy(*buffer, this->constBuffer, bufferLength);
		} else if (this->buffer != NULL) {
			memcpy(*buffer, this->buffer, bufferLength);
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += bufferLength;
		buffer += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

uint32_t SerialBufferAdapter::getSerializedSize() const {
	return bufferLength;
}

ReturnValue_t SerialBufferAdapter::deSerialize(const uint8_t** buffer,
		int32_t* size, bool bigEndian) {
	if (buffer != NULL) {
		if (*size - bufferLength >= 0) {
			*size -= bufferLength;
			memcpy(this->buffer, *buffer, bufferLength);
			buffer += bufferLength;
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return STREAM_TOO_SHORT;
		}
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}
