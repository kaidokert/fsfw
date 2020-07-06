#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
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

template<typename T>
ReturnValue_t SerialBufferAdapter<T>::serialize(uint8_t** buffer, size_t* size,
		size_t maxSize, Endianness streamEndianness) const {
	uint32_t serializedLength = bufferLength;
	if (serializeLength) {
		serializedLength += SerializeAdapter::getSerializedSize(
				&bufferLength);
	}
	if (*size + serializedLength > maxSize) {
		return BUFFER_TOO_SHORT;
	} else {
		if (serializeLength) {
			SerializeAdapter::serialize(&bufferLength, buffer, size,
					maxSize, streamEndianness);
		}
		if (constBuffer != nullptr) {
			memcpy(*buffer, this->constBuffer, bufferLength);
		}
		else if (buffer != nullptr) {
			// This will propably be never reached, constBuffer should always be
			// set if non-const buffer is set.
			memcpy(*buffer, this->buffer, bufferLength);
		}
		else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += bufferLength;
		(*buffer) += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

template<typename T>
size_t SerialBufferAdapter<T>::getSerializedSize() const {
	if (serializeLength) {
		return bufferLength + SerializeAdapter::getSerializedSize(&bufferLength);
	} else {
		return bufferLength;
	}
}

template<typename T>
ReturnValue_t SerialBufferAdapter<T>::deSerialize(const uint8_t** buffer,
		size_t* size, Endianness streamEndianness) {
	//TODO Ignores Endian flag!
	if (buffer != NULL) {
		if(serializeLength){
			T serializedSize = SerializeAdapter::getSerializedSize(
					&bufferLength);
			if(bufferLength + serializedSize <= *size) {
				*buffer +=  serializedSize;
				*size -= serializedSize;
			}
			else {
				return STREAM_TOO_SHORT;
			}
		}
		//No Else If, go on with buffer
		if (bufferLength <= *size) {
			*size -= bufferLength;
			memcpy(this->buffer, *buffer, bufferLength);
			(*buffer) += bufferLength;
			return HasReturnvaluesIF::RETURN_OK;
		}
		else {
			return STREAM_TOO_SHORT;
		}
	}
	else {
		return HasReturnvaluesIF::RETURN_FAILED;
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
		sif::error << "SerialBufferAdapter: Buffers are unitialized!" << std::endl;
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

