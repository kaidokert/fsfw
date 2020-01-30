#ifndef SERIALBUFFERADAPTER2_H_
#define SERIALBUFFERADAPTER2_H_

#include <framework/serialize/SerializeIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

#include <cstring>

/**
 * This adapter provides an interface for SerializeIF to serialize or deserialize
 * buffers with no length header but a known size.
 *
 * Additionally, the buffer length can be serialized too and will be put in front of the serialized buffer.
 *
 * Can be used with SerialLinkedListAdapter by declaring a SerializeElement with
 * SerialElement<SerialBufferAdapter<bufferLengthType(will be uint8_t mostly)>> serialBufferElement.
 * Right now, the SerialBufferAdapter must always be initialized with the buffer and size !
 *
 * \ingroup serialize
 */
template<typename BUFFER_TYPE = uint8_t, typename count_t = uint8_t>
class SerialBufferAdapter2: public SerializeIF {
public:
	/**
	 * Constructor for constant uint8_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter2(void * buffer_, count_t bufferLength_, bool serializeLength_ = false):
				bufferLength(bufferLength_), serializeLength(serializeLength_) {
		determineLengthInBytes(sizeof(BUFFER_TYPE));
		buffer = reinterpret_cast<const uint8_t *>(buffer_);
		constBuffer = NULL;
	}

	SerialBufferAdapter2(const void * buffer_, count_t bufferLength_, bool serializeLength_ = false):
					bufferLength(bufferLength_), serializeLength(serializeLength_) {
		determineLengthInBytes(sizeof(BUFFER_TYPE));
		constBuffer = reinterpret_cast<const uint8_t *>(buffer_);
		buffer = NULL;
	}

	ReturnValue_t serialize(uint8_t ** buffer, uint32_t* size,
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
			memcpy(*buffer, this->buffer, bufferLength);
			*size += bufferLength;
			(*buffer) += bufferLength;
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	uint32_t getSerializedSize() const {
		if (serializeLength) {
			return bufferLength + AutoSerializeAdapter::getSerializedSize(&bufferLength);
		} else {
			return bufferLength;
		}
	}

	ReturnValue_t deSerialize(const uint8_t** buffer,
			int32_t* size, bool bigEndian) {
		//TODO Ignores Endian flag!
		if (buffer != NULL) {
			if(serializeLength){
				// Suggestion (would require removing rest of the block inside this if clause !):
				//ReturnValue_t result = AutoSerializeAdapter::deSerialize(&bufferLength,buffer,size,bigEndian);
				//if (result != HasReturnvaluesIF::RETURN_OK) {
				//	return result;
				//}
				count_t serializedSize = AutoSerializeAdapter::getSerializedSize(
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


	BUFFER_TYPE * getBuffer() {
		return reinterpret_cast<BUFFER_TYPE *>(buffer);
	}

	void setBuffer(void * buffer_, count_t bufferLength_, bool serializeLength_ = false) {
		buffer = buffer_;
		bufferLength = bufferLength_;
		serializeLength = serializeLength_;
		determineLengthInBytes(sizeof(BUFFER_TYPE));
	}

	void setConstBuffer(const void * buffer_, count_t bufferLength_, bool serializeLength_ = false) {
		constBuffer = buffer_;
		bufferLength = bufferLength_;
		serializeLength = serializeLength_;
		determineLengthInBytes(sizeof(BUFFER_TYPE));
	}
private:
	uint8_t * buffer;
	const uint8_t * constBuffer;
	count_t bufferLength;
	bool serializeLength;

	void determineLengthInBytes(uint8_t typeSize) {
		switch(typeSize) {
		case(1): break;
		case(2):
			bufferLength *= 2; break;
		case(4):
			bufferLength *= 4; break;
		case(8):
			bufferLength *= 8; break;
		default:
			error << "Serial Buffer Adapter 2: Invalid type size, assuming regular uint8_t." << std::endl;
			error << "Detected type size: " << (int) typeSize << std::endl;
		}
	}
};

#endif /* SERIALBUFFERADAPTER2_H_ */
