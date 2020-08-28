#include "../serialize/SerialBufferAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include <cstring>

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(const uint8_t* buffer,
		count_t bufferLength, bool serializeLength) :
		serializeLength(serializeLength),
		constBuffer(buffer), buffer(nullptr),
		bufferLength(bufferLength) {}

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(uint8_t* buffer,
		count_t bufferLength, bool serializeLength) :
		serializeLength(serializeLength), constBuffer(buffer), buffer(buffer),
		bufferLength(bufferLength) {}


template<typename count_t>
SerialBufferAdapter<count_t>::~SerialBufferAdapter() {
}

template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::serialize(uint8_t** buffer,
		size_t* size, size_t maxSize, Endianness streamEndianness) const {
	if (serializeLength) {
		ReturnValue_t result = SerializeAdapter::serialize(&bufferLength,
				buffer, size, maxSize, streamEndianness);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}

	if (*size + bufferLength > maxSize) {
		return BUFFER_TOO_SHORT;
	}

	if (this->constBuffer != nullptr) {
		std::memcpy(*buffer, this->constBuffer, bufferLength);
	}
	else if (this->buffer != nullptr) {
		// This will propably be never reached, constBuffer should always be
		// set if non-const buffer is set.
		std::memcpy(*buffer, this->buffer, bufferLength);
	}
	else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	*size += bufferLength;
	(*buffer) += bufferLength;
	return HasReturnvaluesIF::RETURN_OK;

}

template<typename count_t>
size_t SerialBufferAdapter<count_t>::getSerializedSize() const {
	if (serializeLength) {
		return bufferLength + SerializeAdapter::getSerializedSize(&bufferLength);
	} else {
		return bufferLength;
	}
}

template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::deSerialize(const uint8_t** buffer,
		size_t* size, Endianness streamEndianness) {
	if (this->buffer == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	if(serializeLength){
		count_t lengthField = 0;
		ReturnValue_t result = SerializeAdapter::deSerialize(&lengthField,
				buffer, size, streamEndianness);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		if(lengthField > bufferLength) {
			return TOO_MANY_ELEMENTS;
		}
		bufferLength = lengthField;
	}

	if (bufferLength <= *size) {
		*size -= bufferLength;
		std::memcpy(this->buffer, *buffer, bufferLength);
		(*buffer) += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return STREAM_TOO_SHORT;
	}
}

template<typename count_t>
uint8_t * SerialBufferAdapter<count_t>::getBuffer() {
	if(buffer == nullptr) {
		sif::error << "Wrong access function for stored type !"
				 " Use getConstBuffer()." << std::endl;
		return nullptr;
	}
	return buffer;
}

template<typename count_t>
const uint8_t * SerialBufferAdapter<count_t>::getConstBuffer() {
	if(constBuffer == nullptr) {
		sif::error << "SerialBufferAdapter::getConstBuffer:"
				" Buffers are unitialized!" << std::endl;
		return nullptr;
	}
	return constBuffer;
}

template<typename count_t>
void SerialBufferAdapter<count_t>::setBuffer(uint8_t* buffer,
		count_t bufferLength) {
	this->buffer = buffer;
	this->constBuffer = buffer;
	this->bufferLength = bufferLength;
}


//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;
template class SerialBufferAdapter<uint64_t>;

