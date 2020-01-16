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
	SerialBufferAdapter2(BUFFER_TYPE * buffer_, count_t bufferLength_, bool serializeLength_ = false):
				buffer(buffer_),bufferLength(bufferLength_), serializeLength(serializeLength_) {
		determineLengthMultiplier(sizeof(count_t));
		if(std::is_const<BUFFER_TYPE>::value) {
			isConst = true;
		}
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
		//UPDATE: Endian swapper introduced. Must be tested..
		if (buffer != NULL) {
			if(serializeLength){
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
				uint8_t tmp [bufferLength];
				uint8_t * pTmp = tmp;
				if (bigEndian) {
					EndianSwapper::swap<BUFFER_TYPE>(pTmp,*buffer, bufferLength);
				} else {
					pTmp = const_cast<uint8_t *>(*buffer);
				}
				*size -= bufferLength;
				memcpy(const_cast<void *>(reinterpret_cast<const void*>(this->buffer)), pTmp, bufferLength);
				(*buffer) += bufferLength;
				return HasReturnvaluesIF::RETURN_OK;
			} else {
				return STREAM_TOO_SHORT;
			}
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}


	uint8_t * getBuffer() {
		return reinterpret_cast<uint8_t *>(buffer);
	}

	void setBuffer(BUFFER_TYPE * buffer_, count_t bufferLength_, bool serializeLength_ = false) {
		buffer = buffer_;
		bufferLength = bufferLength_;
		serializeLength = serializeLength_;
		determineLengthMultiplier(sizeof(count_t));
	}
private:
	BUFFER_TYPE * buffer;
	count_t bufferLength;
	bool serializeLength;

	bool isConst = false;

	void determineLengthMultiplier(uint8_t typeSize) {
		switch(typeSize) {
		case(2):
			bufferLength *= 2; break;
		case(4):
			bufferLength *= 4; break;
		case(8):
			bufferLength *= 8; break;
		default:
			warning << "Invalid type size, assuming regular uint8_t." << std::endl;
		}
	}
};



#endif /* SERIALBUFFERADAPTER2_H_ */
