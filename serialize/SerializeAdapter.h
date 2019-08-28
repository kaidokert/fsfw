#ifndef SERIALIZEADAPTER_H_
#define SERIALIZEADAPTER_H_

#include <framework/container/IsDerivedFrom.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/EndianSwapper.h>
#include <framework/serialize/SerializeIF.h>
#include <string.h>

/**
 * \ingroup serialize
 */
template<typename T, int>
class SerializeAdapter_ {
public:
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			uint32_t* size, const uint32_t max_size, bool bigEndian) {
		uint32_t ignoredSize = 0;
		if (size == NULL) {
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

	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, int32_t* size,
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

	uint32_t getSerializedSize(const T * object) {
		return sizeof(T);
	}

};

template<typename T>
class SerializeAdapter_<T, 1> {
public:
	ReturnValue_t serialize(const T* object, uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		uint32_t ignoredSize = 0;
		if (size == NULL) {
			size = &ignoredSize;
		}
		return object->serialize(buffer, size, max_size, bigEndian);
	}
	uint32_t getSerializedSize(const T* object) const {
		return object->getSerializedSize();
	}

	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return object->deSerialize(buffer, size, bigEndian);
	}
};

template<typename T>
class SerializeAdapter {
public:
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			uint32_t* size, const uint32_t max_size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.serialize(object, buffer, size, max_size, bigEndian);
	}
	static uint32_t getSerializedSize(const T* object) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.getSerializedSize(object);
	}

	static ReturnValue_t deSerialize(T* object, const uint8_t** buffer,
			int32_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
	}
};


class AutoSerializeAdapter {
public:
	template<typename T>
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			uint32_t* size, const uint32_t max_size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.serialize(object, buffer, size, max_size, bigEndian);
	}
	template<typename T>
	static uint32_t getSerializedSize(const T* object) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.getSerializedSize(object);
	}
	template<typename T>
	static ReturnValue_t deSerialize(T* object, const uint8_t** buffer,
			int32_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
	}
};

#endif /* SERIALIZEADAPTER_H_ */
