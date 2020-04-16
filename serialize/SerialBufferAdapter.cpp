#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <cstring>

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(const void* buffer,
		count_t bufferLength, bool serializeLength) :
		serializeLength(serializeLength),
		constBuffer(static_cast<const uint8_t *>(buffer)), m_buffer(nullptr),
		bufferLength(bufferLength) {

}

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(void* buffer,
		count_t bufferLength, bool serializeLength) :
		serializeLength(serializeLength), bufferLength(bufferLength) {
	uint8_t * member_buffer = static_cast<uint8_t *>(buffer);
	m_buffer = member_buffer;
	constBuffer = member_buffer;
}


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
		} else if (m_buffer != nullptr) {
			memcpy(*buffer, m_buffer, bufferLength);
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
	// (Robin) the one of the buffer? wouldn't that be an issue for serialize
	// as well? SerialFixedArrayListAdapter implements swapping of buffer
	// fields (if buffer type is not uint8_t)
	if (buffer != nullptr) {
		if(serializeLength) {
			count_t serializedSize = AutoSerializeAdapter::getSerializedSize(
					&bufferLength);
			if(bufferLength + serializedSize >= *size) {
				*buffer +=  serializedSize;
				*size -= serializedSize;
			}
			else {
				return STREAM_TOO_SHORT;
			}
		}
		//No Else If, go on with buffer
		if (bufferLength >= *size) {
			*size -= bufferLength;
			memcpy(m_buffer, *buffer, bufferLength);
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
	if(m_buffer == nullptr) {
		error << "Wrong access function for stored type !"
				 " Use getConstBuffer()" << std::endl;
		return nullptr;
	}
	return m_buffer;
}

template<typename count_t>
const uint8_t * SerialBufferAdapter<count_t>::getConstBuffer() {
	if(constBuffer == nullptr) {
		error << "Wrong access function for stored type !"
				 " Use getBuffer()" << std::endl;
		return nullptr;
	}
	return constBuffer;
}

template<typename count_t>
void SerialBufferAdapter<count_t>::setBuffer(void * buffer,
		count_t buffer_length) {
	m_buffer = static_cast<uint8_t *>(buffer);
	bufferLength = buffer_length;
}


//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;

