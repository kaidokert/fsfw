#include <framework/serialize/SerialBufferAdapter.h>
#include <cstring>

template<typename T>
SerialBufferAdapter<T>::SerialBufferAdapter(const uint8_t* buffer,
		T bufferLength, bool serializeLength) :
		serializeLength(serializeLength), constBuffer(buffer), buffer(NULL), bufferLength(
				bufferLength) {
}

template<typename T>
SerialBufferAdapter<T>::SerialBufferAdapter(uint8_t* buffer, T bufferLength,
		bool serializeLength) :
		serializeLength(serializeLength), constBuffer(NULL), buffer(buffer), bufferLength(
				bufferLength) {
}

template<typename T>
SerialBufferAdapter<T>::SerialBufferAdapter(uint32_t* buffer,
		T bufferLength, bool serializeLength) :
		serializeLength(serializeLength), constBuffer(NULL), buffer(reinterpret_cast<uint8_t *>(buffer)),
		bufferLength(bufferLength*4) {
}

template<typename T>
SerialBufferAdapter<T>::~SerialBufferAdapter() {
}

template<typename T>
ReturnValue_t SerialBufferAdapter<T>::serialize(uint8_t** buffer, uint32_t* size,
		const uint32_t max_size, bool bigEndian) const {
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
		if (this->constBuffer != NULL) {
			memcpy(*buffer, this->constBuffer, bufferLength);
		} else if (this->buffer != NULL) {
			memcpy(*buffer, this->buffer, bufferLength);
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += bufferLength;
		(*buffer) += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

template<typename T>
uint32_t SerialBufferAdapter<T>::getSerializedSize() const {
	if (serializeLength) {
		return bufferLength + AutoSerializeAdapter::getSerializedSize(&bufferLength);
	} else {
		return bufferLength;
	}
}
template<typename T>
ReturnValue_t SerialBufferAdapter<T>::deSerialize(const uint8_t** buffer,
		int32_t* size, bool bigEndian) {
	//TODO Ignores Endian flag!
	if (buffer != NULL) {
		if(serializeLength){
			T serializedSize = AutoSerializeAdapter::getSerializedSize(
					&bufferLength);
			if((*size - bufferLength - serializedSize) >= 0){
				*buffer +=  serializedSize;
				*size -= serializedSize;
			}else{
				return STREAM_TOO_SHORT;
			}
		}
		//No Else If, go on with buffer
		if (*size - bufferLength >= 0) {
			*size -= bufferLength;
			memcpy(this->buffer, *buffer, bufferLength);
			(*buffer) += bufferLength;
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return STREAM_TOO_SHORT;
		}
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template<typename T>
uint8_t * SerialBufferAdapter<T>::getBuffer() {
	return buffer;
}

//template<typename T>
//void SerialBufferAdapter<T>::setBuffer(uint8_t * buffer_) {
//	buffer = buffer_;
//}

//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;

