#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <cstring>

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(const void* buffer,
		count_t bufferLength, bool serializeLength) :
		m_serialize_length(serializeLength),
		m_const_buffer(static_cast<const uint8_t *>(buffer)), m_buffer(nullptr),
		m_buffer_length(bufferLength) {

}

template<typename count_t>
SerialBufferAdapter<count_t>::SerialBufferAdapter(void* buffer, count_t bufferLength,
		bool serializeLength) :
		m_serialize_length(serializeLength), m_buffer_length(bufferLength) {
	uint8_t * member_buffer = static_cast<uint8_t *>(buffer);
	m_buffer = member_buffer;
	m_const_buffer = member_buffer;
}


template<typename count_t>
SerialBufferAdapter<count_t>::~SerialBufferAdapter() {
}

template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::serialize(uint8_t** buffer, uint32_t* size,
		const uint32_t max_size, bool bigEndian) const {
	uint32_t serializedLength = m_buffer_length;
	if (m_serialize_length) {
		serializedLength += AutoSerializeAdapter::getSerializedSize(
				&m_buffer_length);
	}
	if (*size + serializedLength > max_size) {
		return BUFFER_TOO_SHORT;
	} else {
		if (m_serialize_length) {
			AutoSerializeAdapter::serialize(&m_buffer_length, buffer, size,
					max_size, bigEndian);
		}
		if (m_const_buffer != nullptr) {
			memcpy(*buffer, m_const_buffer, m_buffer_length);
		} else if (m_buffer != nullptr) {
			memcpy(*buffer, m_buffer, m_buffer_length);
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += m_buffer_length;
		(*buffer) += m_buffer_length;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

template<typename count_t>
uint32_t SerialBufferAdapter<count_t>::getSerializedSize() const {
	if (m_serialize_length) {
		return m_buffer_length + AutoSerializeAdapter::getSerializedSize(&m_buffer_length);
	} else {
		return m_buffer_length;
	}
}
template<typename count_t>
ReturnValue_t SerialBufferAdapter<count_t>::deSerialize(const uint8_t** buffer,
		int32_t* size, bool bigEndian) {
	//TODO Ignores Endian flag!
	if (buffer != NULL) {
		if(m_serialize_length){
			// Suggestion (would require removing rest of the block inside this if clause !):
			//ReturnValue_t result = AutoSerializeAdapter::deSerialize(&bufferLength,buffer,size,bigEndian);
			//if (result != HasReturnvaluesIF::RETURN_OK) {
			//	return result;
			//}
			count_t serializedSize = AutoSerializeAdapter::getSerializedSize(
					&m_buffer_length);
			if((*size - m_buffer_length - serializedSize) >= 0){
				*buffer +=  serializedSize;
				*size -= serializedSize;
			}else{
				return STREAM_TOO_SHORT;
			}
		}
		//No Else If, go on with buffer
		if (*size - m_buffer_length >= 0) {
			*size -= m_buffer_length;
			memcpy(m_buffer, *buffer, m_buffer_length);
			(*buffer) += m_buffer_length;
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return STREAM_TOO_SHORT;
		}
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template<typename count_t>
uint8_t * SerialBufferAdapter<count_t>::getBuffer() {
	if(m_buffer == nullptr) {
		error << "Wrong access function for stored type ! Use getConstBuffer()" << std::endl;
		return nullptr;
	}
	return m_buffer;
}

template<typename count_t>
const uint8_t * SerialBufferAdapter<count_t>::getConstBuffer() {
	if(m_const_buffer == nullptr) {
		error << "Wrong access function for stored type ! Use getBuffer()" << std::endl;
		return nullptr;
	}
	return m_const_buffer;
}

template<typename count_t>
void SerialBufferAdapter<count_t>::setBuffer(void * buffer, count_t buffer_length) {
	m_buffer = static_cast<uint8_t *>(buffer);
	m_buffer_length = buffer_length;
}


//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;

