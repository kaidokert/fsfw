#ifndef FSFW_SERIALIZE_SERIALIZEADAPTER_H_
#define FSFW_SERIALIZE_SERIALIZEADAPTER_H_

#include "EndianConverter.h"
#include "SerializeIF.h"

#include "../container/IsDerivedFrom.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include <cstring>

 /**
 * @brief These adapters provides an interface to use the SerializeIF functions
 * 		  with arbitrary template objects to facilitate and simplify the
 * 		  serialization of classes with different multiple different data types
 * 		  into buffers and vice-versa.
 * @details
 * The correct serialization or deserialization function is chosen at
 * compile time with template type deduction.
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
				size_t *size, size_t max_size,
				SerializeIF::Endianness streamEndianness) {
			size_t ignoredSize = 0;
			if (size == nullptr) {
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
				std::memcpy(*buffer, &tmp, sizeof(T));
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
				std::memcpy(&tmp, *buffer, sizeof(T));
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
			if (size == nullptr) {
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
