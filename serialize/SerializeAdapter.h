#ifndef _FSFW_SERIALIZE_SERIALIZEADAPTER_H_
#define _FSFW_SERIALIZE_SERIALIZEADAPTER_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "EndianConverter.h"
#include "SerializeIF.h"
#include <cstddef>
#include <type_traits>

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
	/***
	 * This function can be used to serialize a trivial copy-able type or a
	 * child of SerializeIF.
	 * The right template to be called is determined in the function itself.
	 * For objects of non trivial copy-able type this function is almost never
	 * called by the user directly. Instead helpers for specific types like
	 * SerialArrayListAdapter or SerialLinkedListAdapter is the right choice here.
	 *
	 * @param[in] object Object to serialize, the used type is deduced from this pointer
	 * @param[in/out] buffer Buffer to serialize into. Will be moved by the function.
	 * @param[in/out] size Size of current written buffer. Will be incremented by the function.
	 * @param[in] maxSize Max size of Buffer
	 * @param[in] streamEndianness Endianness of serialized element as in according to SerializeIF::Endianness
	 * @return
	 * 		- @c BUFFER_TOO_SHORT The given buffer in is too short
	 * 		- @c RETURN_FAILED Generic Error
	 * 		- @c RETURN_OK Successful serialization
	 */
	template<typename T>
	static ReturnValue_t serialize(const T *object, uint8_t **buffer,
			size_t *size, size_t maxSize,
			SerializeIF::Endianness streamEndianness) {
		InternalSerializeAdapter<T, std::is_base_of<SerializeIF, T>::value> adapter;
		return adapter.serialize(object, buffer, size, maxSize,
				streamEndianness);
	}
	/**
	 * Function to return the serialized size of the object in the pointer.
	 * May be a trivially copy-able object or a Child of SerializeIF
	 *
	 * @param object Pointer to Object
	 * @return Serialized size of object
	 */
	template<typename T>
	static size_t getSerializedSize(const T *object){
		InternalSerializeAdapter<T, std::is_base_of<SerializeIF, T>::value> adapter;
		return adapter.getSerializedSize(object);
	}
	/**
	 * @brief
	 * Deserializes a object from a given buffer of given size.
	 * Object Must be trivially copy-able or a child of SerializeIF.
	 *
	 * @details
	 * Buffer will be moved to the current read location. Size will be decreased by the function.
	 *
	 * @param[in/out] buffer Buffer to deSerialize from. Will be moved by the function.
	 * @param[in/out] size Remaining size of the buffer to read from. Will be decreased by function.
	 * @param[in] streamEndianness Endianness as in according to SerializeIF::Endianness
	 * @return
	 * 	- @c STREAM_TOO_SHORT The input stream is too short to deSerialize the object
	 * 	- @c TOO_MANY_ELEMENTS The buffer has more inputs than expected
	 * 	- @c RETURN_FAILED Generic Error
	 * 	- @c RETURN_OK Successful deserialization
	 */
	template<typename T>
	static ReturnValue_t deSerialize(T *object, const uint8_t **buffer,
			size_t *size, SerializeIF::Endianness streamEndianness) {
		InternalSerializeAdapter<T, std::is_base_of<SerializeIF, T>::value> adapter;
		return adapter.deSerialize(object, buffer, size, streamEndianness);
	}
private:
	/**
	 * Internal template to deduce the right function calls at compile time
	 */
	template<typename T, bool> class InternalSerializeAdapter;

	/**
	 * Template to be used if T is not a child of SerializeIF
	 *
	 * @tparam T T must be trivially_copyable
	 */
	template<typename T>
	class InternalSerializeAdapter<T, false> {
		static_assert (std::is_trivially_copyable<T>::value,
				"If a type needs to be serialized it must be a child of "
				"SerializeIF or trivially copy-able");
	public:
		static ReturnValue_t serialize(const T *object, uint8_t **buffer,
				size_t *size, size_t max_size,
				SerializeIF::Endianness streamEndianness) {
			size_t ignoredSize = 0;
			if (size == nullptr) {
				size = &ignoredSize;
			}
			// Check remaining size is large enough and check integer
			// overflow of *size
			size_t newSize = sizeof(T) + *size;
			if ((newSize <= max_size) and (newSize > *size)) {
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

	/**
	 * Template for objects that inherit from SerializeIF
	 *
	 * @tparam T A child of SerializeIF
	 */
	template<typename T>
	class InternalSerializeAdapter<T, true> {
	public:
		ReturnValue_t serialize(const T *object, uint8_t **buffer, size_t *size,
				size_t max_size,
				SerializeIF::Endianness streamEndianness) const {
			size_t ignoredSize = 0;
			if (size == nullptr) {
				size = &ignoredSize;
			}
			return object->serialize(buffer, size, max_size, streamEndianness);
		}
		size_t getSerializedSize(const T *object) const {
			return object->getSerializedSize();
		}

		ReturnValue_t deSerialize(T *object, const uint8_t **buffer,
				size_t *size, SerializeIF::Endianness streamEndianness) {
			return object->deSerialize(buffer, size, streamEndianness);
		}
	};
};

#endif /* _FSFW_SERIALIZE_SERIALIZEADAPTER_H_ */
