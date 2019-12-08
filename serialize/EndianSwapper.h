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
	EndianSwapper() {
	}
	;
public:
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
	static void swap(uint8_t* out, const uint8_t* in, uint32_t size) {
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
};

#endif /* ENDIANSWAPPER_H_ */
