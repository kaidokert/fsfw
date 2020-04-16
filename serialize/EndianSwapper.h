#ifndef ENDIANSWAPPER_H_
#define ENDIANSWAPPER_H_

#include <framework/osal/Endiness.h>
#include <cstring>
#include <iostream>

/**
 * @brief Can be used to swap endianness of data
 *        into big endian
 */
class EndianSwapper {
private:
	EndianSwapper() {};
public:
	/**
	 * Swap the endianness of a variable with arbitrary type
	 * @tparam T Type of variable
	 * @param in variable
	 * @return Variable with swapped endianness
	 */
	template<typename T>
	static T swap(T in) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
		T tmp;
		uint8_t *pointerOut = (uint8_t *) &tmp;
		uint8_t *pointerIn = (uint8_t *) &in;
		for (uint8_t count = 0; count < sizeof(T); count++) {
			pointerOut[sizeof(T) - count - 1] = pointerIn[count];
		}
		return tmp;
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
		return in;
#else
#error Unknown Byte Order
#endif
	}

	/**
	 * Swap the endianness of a buffer.
	 * @param out
	 * @param in
	 * @param size
	 */
	static void swap(uint8_t* out, const uint8_t* in, size_t size) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
		for (uint8_t count = 0; count < size; count++) {
			out[size - count - 1] = in[count];
		}
		return;
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
		memcpy(out, in, size);
		return;
#endif
	}

	/**
	 * Swap endianness of buffer entries
	 * Template argument specifies buffer type. The number of entries
	 * (not the buffer size!) must be supplied
	 * @param out
	 * @param in
	 * @param size Number of buffer entries (not size of buffer in bytes!)
	 */
	template<typename T>
	static void swap(T * out, const T * in, uint32_t entries) {
#ifndef BYTE_ORDER_SYSTEM
#error BYTE_ORDER_SYSTEM not defined
#elif BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
		const uint8_t * in_buffer = reinterpret_cast<const uint8_t *>(in);
		uint8_t * out_buffer = reinterpret_cast<uint8_t *>(out);
		for (uint8_t count = 0; count < entries; count++) {
			for(uint8_t i = 0; i < sizeof(T);i++) {
				out_buffer[sizeof(T)* (count + 1) - i - 1] =
						in_buffer[count * sizeof(T) + i];
			}
		}
		return;
#elif BYTE_ORDER_SYSTEM == BIG_ENDIAN
		memcpy(out, in, size*sizeof(T));
		return;
#endif
	}
};

#endif /* ENDIANSWAPPER_H_ */
