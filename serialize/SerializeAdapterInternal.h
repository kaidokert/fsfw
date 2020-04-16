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
 * This template specialization will be chosen for fundamental types or
 * anything else not implementing SerializeIF, based on partial
 * template specialization.
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
	    // function eventuelly serializes structs here.
	    // does this work on every architecture?
	    // static_assert(std::is_fundamental<T>::value);
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
	 * @param buffer contains the data. Non-Const pointer to non-const
	 * 				 pointer to const data.
	 * @param size Size to deSerialize. wil be decremented by sizeof(T)
	 * @param bigEndian Specify endianness
	 * @return
	 */
	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, size_t* size,
			bool bigEndian) {
		T tmp;
		if (*size >= sizeof(T)) {
			*size -= sizeof(T);
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
 * SerializeIF, based on partial template specialization.
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

	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, size_t* size,
			bool bigEndian) {
		return object->deSerialize(buffer, size, bigEndian);
	}
};

#endif /* FRAMEWORK_SERIALIZE_SERIALIZEADAPTERINTERNAL_H_ */
