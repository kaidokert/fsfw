/**
 * @file 	SerializeAdapterInternal.h
 *
 * @date	13.04.2020
 * @author	R. Mueller
 */

#ifndef FRAMEWORK_SERIALIZE_SERIALIZEADAPTERINTERNAL_H_
#define FRAMEWORK_SERIALIZE_SERIALIZEADAPTERINTERNAL_H_
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/container/IsDerivedFrom.h>
#include <framework/serialize/EndianSwapper.h>

/**
 * This template specialization will be chosen for fundamental types.
 * @tparam T
 * @tparam
 */
template<typename T, int>
class SerializeAdapter_ {
public:
	/**
	 *
	 * @param object
	 * @param buffer
	 * @param size
	 * @param max_size
	 * @param bigEndian
	 * @return
	 */
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			size_t* size, const size_t max_size, bool bigEndian) {
		size_t ignoredSize = 0;
		if (size == nullptr) {
			size = &ignoredSize;
		}
		if (sizeof(T) + *size <= max_size) {
			T tmp;
			if (bigEndian) {
				tmp = EndianSwapper::swap<T>(*object);
			} else {
				tmp = *object;
			}
			memcpy(*buffer, &tmp, sizeof(T));
			*size += sizeof(T);
			(*buffer) += sizeof(T);
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return SerializeIF::BUFFER_TOO_SHORT;
		}
	}

	/**
	 * Deserialize buffer into object
	 * @param object [out] Object to be deserialized with buffer data
	 * @param buffer buffer containing the data. Non-Const pointer to non-const
	 * 						pointer to const buffer.
	 * @param size int32_t type to allow value to be values smaller than 0,
	 * 						needed for range/size checking
	 * @param bigEndian Specify endianness
	 * @return
	 */
	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, ssize_t* size,
			bool bigEndian) {
		T tmp;
		*size -= sizeof(T);
		if (*size >= 0) {
			memcpy(&tmp, *buffer, sizeof(T));
			if (bigEndian) {
				*object = EndianSwapper::swap<T>(tmp);
			} else {
				*object = tmp;
			}
			*buffer += sizeof(T);
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return SerializeIF::STREAM_TOO_SHORT;
		}
	}

	size_t getSerializedSize(const T * object) {
		return sizeof(T);
	}
};

/**
 * This template specialization will be chosen for class derived from
 * SerializeIF.
 * @tparam T
 * @tparam
 */
template<typename T>
class SerializeAdapter_<T, true> {
public:
	ReturnValue_t serialize(const T* object, uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		size_t ignoredSize = 0;
		if (size == NULL) {
			size = &ignoredSize;
		}
		return object->serialize(buffer, size, max_size, bigEndian);
	}

	size_t getSerializedSize(const T* object) const {
		return object->getSerializedSize();
	}

	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, ssize_t* size,
			bool bigEndian) {
		return object->deSerialize(buffer, size, bigEndian);
	}
};

#endif /* FRAMEWORK_SERIALIZE_SERIALIZEADAPTERINTERNAL_H_ */
