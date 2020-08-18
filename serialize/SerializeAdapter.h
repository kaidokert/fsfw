#ifndef SERIALIZEADAPTER_H_
#define SERIALIZEADAPTER_H_

#include "../container/IsDerivedFrom.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serialize/EndianConverter.h"
#include "../serialize/SerializeIF.h"
#include <type_traits>

 /**
 * @brief These adapters provides an interface to use the SerializeIF functions
 * 		  with arbitrary template objects to facilitate and simplify the
 * 		  serialization of classes with different multiple different data types
 * 		  into buffers and vice-versa.
 * @details
 *
 * A report class is converted into a TM buffer. The report class implements a
 * serialize functions and calls the AutoSerializeAdapter::serialize function
 * repeatedly on all object data fields. The getSerializedSize function is
 * implemented by calling the AutoSerializeAdapter::getSerializedSize function
 * repeatedly on all data fields.
 *
 * The AutoSerializeAdapter functions can also be used as an alternative to
 * memcpy to retrieve data out of a buffer directly into a class variable
 * with data type T while being able to specify endianness. The boolean
 * bigEndian specifies whether an endian swap is performed on the data before
 * serialization or deserialization.
 *
 * There are three ways to retrieve data out of a buffer to be used in the FSFW
 * to use regular aligned (big endian) data. Examples:
 *
 *   1. Use the AutoSerializeAdapter::deSerialize function
 *		The pointer *buffer will be incremented automatically by the typeSize
 *		 of the object, so this function can be called on &buffer repeatedly
 *		without adjusting pointer position. Set bigEndian parameter to true
 *		to perform endian swapping, if necessary
 *		@code
 *   	uint16_t data;
 *   	int32_t dataLen = sizeof(data);
 *   	ReturnValue_t result =
 *   			AutoSerializeAdapter::deSerialize(&data,&buffer,&dataLen,true);
 *   	@endcode
 *
 *   2. Perform a bitshift operation. Watch for for endianness:
 *		@code
 *   	uint16_t data;
 *   	data = buffer[targetByte1] << 8 | buffer[targetByte2];
 *   	data = EndianSwapper::swap(data); //optional, or swap order above
 *   	@endcode
 *
 *   3. memcpy or std::copy can also be used, but watch out if system
 *   	endianness is different from required data endianness.
 *   	Perform endian-swapping if necessary.
 *		@code
 *   	uint16_t data;
 *   	memcpy(&data,buffer + positionOfTargetByte1,sizeof(data));
 *   	data = EndianSwapper::swap(data); //optional
 *   	@endcode
 *
 * When serializing for downlink, the packets are generally serialized assuming
 * big endian data format like seen in TmPacketStored.cpp for example.
 *
 * @ingroup serialize
 */

class SerializeAdapter {
public:
	template<typename T>
	static ReturnValue_t serialize(const T *object, uint8_t **buffer,
			size_t *size, size_t maxSize, SerializeIF::Endianness streamEndianness) {
		InternalSerializeAdapter<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.serialize(object, buffer, size, maxSize,
				streamEndianness);
	}
	template<typename T>
	static uint32_t getSerializedSize(const T *object) {
		InternalSerializeAdapter<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.getSerializedSize(object);
	}
	template<typename T>
	static ReturnValue_t deSerialize(T *object, const uint8_t **buffer,
			size_t *size, SerializeIF::Endianness streamEndianness) {
		InternalSerializeAdapter<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, streamEndianness);
	}
private:
	template<typename T, int>
	class InternalSerializeAdapter {
	public:
		static ReturnValue_t serialize(const T *object, uint8_t **buffer,
				size_t *size, size_t max_size, SerializeIF::Endianness streamEndianness) {
			size_t ignoredSize = 0;
			if (size == NULL) {
				size = &ignoredSize;
			}
			//TODO check integer overflow of *size
			if (sizeof(T) + *size <= max_size) {
				T tmp;
				switch (streamEndianness) {
				case SerializeIF::Endianness::BIG:
					tmp = EndianConverter::convertBigEndian<T>(*object);
					break;
				case SerializeIF::Endianness::LITTLE:
					tmp = EndianConverter::convertLittleEndian<T>(*object);
					break;
				default:
				case SerializeIF::Endianness::MACHINE:
					tmp = *object;
					break;
				}
				memcpy(*buffer, &tmp, sizeof(T));
				*size += sizeof(T);
				(*buffer) += sizeof(T);
				return HasReturnvaluesIF::RETURN_OK;
			} else {
				return SerializeIF::BUFFER_TOO_SHORT;
			}
		}

		ReturnValue_t deSerialize(T *object, const uint8_t **buffer,
				size_t *size, SerializeIF::Endianness streamEndianness) {
			T tmp;
			if (*size >= sizeof(T)) {
				*size -= sizeof(T);
				memcpy(&tmp, *buffer, sizeof(T));
				switch (streamEndianness) {
				case SerializeIF::Endianness::BIG:
					*object = EndianConverter::convertBigEndian<T>(tmp);
					break;
				case SerializeIF::Endianness::LITTLE:
					*object = EndianConverter::convertLittleEndian<T>(tmp);
					break;
				default:
				case SerializeIF::Endianness::MACHINE:
					*object = tmp;
					break;
				}

				*buffer += sizeof(T);
				return HasReturnvaluesIF::RETURN_OK;
			} else {
				return SerializeIF::STREAM_TOO_SHORT;
			}
		}

		uint32_t getSerializedSize(const T *object) {
			return sizeof(T);
		}

	};

	template<typename T>
	class InternalSerializeAdapter<T, 1> {
	public:
		ReturnValue_t serialize(const T *object, uint8_t **buffer,
				size_t *size, size_t max_size,
				SerializeIF::Endianness streamEndianness) const {
			size_t ignoredSize = 0;
			if (size == NULL) {
				size = &ignoredSize;
			}
			return object->serialize(buffer, size, max_size, streamEndianness);
		}
		uint32_t getSerializedSize(const T *object) const {
			return object->getSerializedSize();
		}

		ReturnValue_t deSerialize(T *object, const uint8_t **buffer,
				size_t *size, SerializeIF::Endianness streamEndianness) {
			return object->deSerialize(buffer, size, streamEndianness);
		}
	};
};

#endif /* SERIALIZEADAPTER_H_ */
