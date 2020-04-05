#ifndef SERIALIZEADAPTER_H_
#define SERIALIZEADAPTER_H_

#include <framework/container/IsDerivedFrom.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/EndianSwapper.h>
#include <framework/serialize/SerializeIF.h>
#include <string.h>

 /**
 * @brief This adapter provides an interface to use the SerializeIF functions
 * 		  with arbitrary template objects to facilitate and simplify the serialization of classes
 * 		  with different multiple different data types into buffers and vice-versa.
 * @details
 * Examples:
 * A report class is converted into a TM buffer. The report class implements a serialize functions and calls
 * the AutoSerializeAdapter::serialize function repeatedly on all object data fields.
 * The getSerializedSize function is implemented by calling the
 * AutoSerializeAdapter::getSerializedSize function repeatedly on all data fields.
 *
 * The AutoSerializeAdapter functions can also be used as an alternative to memcpy
 * to retrieve data out of a buffer directly into a class variable with data type T while being able to specify endianness.
 * The boolean bigEndian specifies whether an endian swap is performed on the data before
 * serialization or deserialization.
 *
 * If the target architecture is little endian (ARM), any data types created might
 * have the wrong endiness if they are to be used for the FSFW.
 * There are three ways to retrieve data out of a buffer to be used in the FSFW
 * to use regular aligned (big endian) data.
 * This can also be applied to uint32_t and uint64_t:
 *
 *   1. Use the AutoSerializeAdapter::deSerialize function
 *		The pointer *buffer will be incremented automatically by the typeSize of the object,
 *		so this function can be called on &buffer repeatedly without adjusting pointer position.
 *		Set bigEndian parameter to true to perform endian swapping.
 *
 *   	uint16_t data;
 *   	int32_t dataLen = sizeof(data);
 *   	ReturnValue_t result = AutoSerializeAdapter::deSerialize(&data,&buffer,&dataLen,true);
 *
 *   2. Perform a bitshift operation. Perform endian swapping if necessary:
 *
 *   	uint16_t data;
 *   	data = buffer[targetByte1] << 8 | buffer[targetByte2];
 *   	data = EndianSwapper::swap(data);
 *
 *   3. Memcpy can be used when data is little-endian. Perform endian-swapping if necessary.
 *
 *   	uint16_t data;
 *   	memcpy(&data,buffer + positionOfTargetByte1,sizeof(data));
 *   	data = EndianSwapper::swap(data);
 *
 * When serializing for downlink, the packets are generally serialized assuming big endian data format
 * like seen in TmPacketStored.cpp for example.
 *
 * @ingroup serialize
 */
template<typename T, int>
class SerializeAdapter_ {
public:
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			size_t* size, const size_t max_size, bool bigEndian) {
		size_t ignoredSize = 0;
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

	/**
	 * Deserialize buffer into object
	 * @param object [out] Object to be deserialized with buffer data
	 * @param buffer buffer containing the data. Non-Const pointer to non-const pointer to const buffer.
	 * @param size int32_t type to allow value to be values smaller than 0, needed for range/size checking
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

	uint32_t getSerializedSize(const T * object) {
		return sizeof(T);
	}

};

template<typename T>
class SerializeAdapter_<T, 1> {
public:
	ReturnValue_t serialize(const T* object, uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		size_t ignoredSize = 0;
		if (size == NULL) {
			size = &ignoredSize;
		}
		return object->serialize(buffer, size, max_size, bigEndian);
	}
	uint32_t getSerializedSize(const T* object) const {
		return object->getSerializedSize();
	}

	ReturnValue_t deSerialize(T* object, const uint8_t** buffer, ssize_t* size,
			bool bigEndian) {
		return object->deSerialize(buffer, size, bigEndian);
	}
};

template<typename T>
class SerializeAdapter {
public:
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			size_t* size, const size_t max_size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.serialize(object, buffer, size, max_size, bigEndian);
	}
	static uint32_t getSerializedSize(const T* object) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.getSerializedSize(object);
	}

	static ReturnValue_t deSerialize(T* object, const uint8_t** buffer,
			ssize_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
	}
};

// No type specification necessary here.
class AutoSerializeAdapter {
public:
	template<typename T>
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			size_t* size, const size_t max_size, bool bigEndian) {
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
			ssize_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
	}
};

#endif /* SERIALIZEADAPTER_H_ */
