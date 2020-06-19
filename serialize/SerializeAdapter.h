#ifndef SERIALIZEADAPTER_H_
#define SERIALIZEADAPTER_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/SerializeIF.h>
#include <framework/serialize/SerializeAdapterInternal.h>
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

// No type specification necessary here.
class AutoSerializeAdapter {
public:
    /**
     * Serialize object into buffer.
     * @tparam T Type of object.
     * @param object Object to serialize
     * @param buffer
     * Serialize into this buffer, pointer to pointer has to be passed,
     * *buffer will be incremented automatically.
     * @param size [out]
     * Update passed size value, will be incremented by serialized size
     * @param max_size
     * Maximum size for range checking
     * @param bigEndian
     * Set to true if host-to-network conversion or vice-versa is needed
     * @return
     */
	template<typename T>
	static ReturnValue_t serialize(const T* object, uint8_t** buffer,
			size_t* size, const size_t max_size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.serialize(object, buffer, size, max_size, bigEndian);
	}
	template<typename T>
	static size_t getSerializedSize(const T* object) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.getSerializedSize(object);
	}
	template<typename T>
	static ReturnValue_t deSerialize(T* object, const uint8_t** buffer,
			size_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
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
			size_t* size, bool bigEndian) {
		SerializeAdapter_<T, IsDerivedFrom<T, SerializeIF>::Is> adapter;
		return adapter.deSerialize(object, buffer, size, bigEndian);
	}
};

#endif /* SERIALIZEADAPTER_H_ */
