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

template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::serialize(uint8_t** buffer,
		size_t* size, const size_t max_size, bool bigEndian) const {
	uint32_t serializedLength = bufferLength;
	if (serializeLength) {
		serializedLength += AutoSerializeAdapter::getSerializedSize(
				&bufferLength);
	}
	if (*size + serializedLength > max_size) {
		return BUFFER_TOO_SHORT;
	} else {
		if (serializeLength) {
			AutoSerializeAdapter::serialize(&bufferLength, buffer, size,
					max_size, bigEndian);
		}
		if (constBuffer != nullptr) {
			memcpy(*buffer, constBuffer, bufferLength);
		} else if (buffer != nullptr) {
			memcpy(*buffer, buffer, bufferLength);
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += bufferLength;
		(*buffer) += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

template<typename count_t>
size_t SerialBufferAdapter<count_t>::getSerializedSize() const {
	if (serializeLength) {
		return bufferLength + AutoSerializeAdapter::getSerializedSize(&bufferLength);
	} else {
		return bufferLength;
	}
}

template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::deSerialize(const uint8_t** buffer,
		size_t* size, bool bigEndian) {
	//TODO Ignores Endian flag!
	if (buffer != nullptr) {
		if(serializeLength) {
			count_t serializedSize = AutoSerializeAdapter::getSerializedSize(
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
				 " Use getConstBuffer()" << std::endl;
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
		count_t buffer_length) {
	this->buffer = buffer;
	bufferLength = buffer_length;
}


//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;

