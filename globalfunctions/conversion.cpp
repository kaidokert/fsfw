#include <framework/globalfunctions/conversion.h>
//TODO REMOVE. Needed because of BYTE_ORDER
#include <framework/osal/Endiness.h>
#include <cstring>



//SHOULDDO: This shall be optimized (later)!
void convertToByteStream( uint16_t value, uint8_t* buffer, uint32_t* size ) {
	buffer[0] = (value & 0xFF00) >> 8;
	buffer[1] = (value & 0x00FF);
	*size += 2;
}

void convertToByteStream( uint32_t value, uint8_t* buffer, uint32_t* size ) {
	buffer[0] = (value & 0xFF000000) >> 24;
	buffer[1] = (value & 0x00FF0000) >> 16;
	buffer[2] = (value & 0x0000FF00) >> 8;
	buffer[3] = (value & 0x000000FF);
	*size +=4;
}

void convertToByteStream( int16_t value, uint8_t* buffer, uint32_t* size ) {
	buffer[0] = (value & 0xFF00) >> 8;
	buffer[1] = (value & 0x00FF);
	*size += 2;
}

void convertToByteStream( int32_t value, uint8_t* buffer, uint32_t* size ) {
	buffer[0] = (value & 0xFF000000) >> 24;
	buffer[1] = (value & 0x00FF0000) >> 16;
	buffer[2] = (value & 0x0000FF00) >> 8;
	buffer[3] = (value & 0x000000FF);
	*size += 4;
}

//void convertToByteStream( uint64_t value, uint8_t* buffer, uint32_t* size ) {
//	buffer[0] = (value & 0xFF00000000000000) >> 56;
//	buffer[1] = (value & 0x00FF000000000000) >> 48;
//	buffer[2] = (value & 0x0000FF0000000000) >> 40;
//	buffer[3] = (value & 0x000000FF00000000) >> 32;
//	buffer[4] = (value & 0x00000000FF000000) >> 24;
//	buffer[5] = (value & 0x0000000000FF0000) >> 16;
//	buffer[6] = (value & 0x000000000000FF00) >> 8;
//	buffer[7] = (value & 0x00000000000000FF);
//	*size+=8;
//}
//
//void convertToByteStream( int64_t value, uint8_t* buffer, uint32_t* size ) {
//	buffer[0] = (value & 0xFF00000000000000) >> 56;
//	buffer[1] = (value & 0x00FF000000000000) >> 48;
//	buffer[2] = (value & 0x0000FF0000000000) >> 40;
//	buffer[3] = (value & 0x000000FF00000000) >> 32;
//	buffer[4] = (value & 0x00000000FF000000) >> 24;
//	buffer[5] = (value & 0x0000000000FF0000) >> 16;
//	buffer[6] = (value & 0x000000000000FF00) >> 8;
//	buffer[7] = (value & 0x00000000000000FF);
//	*size+=8;
//}

void convertToByteStream( float in_value, uint8_t* buffer, uint32_t* size ) {
#ifndef BYTE_ORDER
	#error BYTE_ORDER not defined
#elif BYTE_ORDER == LITTLE_ENDIAN
	union float_union {
		float value;
		uint8_t chars[4];
	};
	float_union temp;
	temp.value = in_value;
	buffer[0] = temp.chars[3];
	buffer[1] = temp.chars[2];
	buffer[2] = temp.chars[1];
	buffer[3] = temp.chars[0];
	*size += 4;
#elif BYTE_ORDER == BIG_ENDIAN
	memcpy(buffer, &in_value, sizeof(in_value));
	*size += sizeof(in_value);
#endif
}

void convertToByteStream( double in_value, uint8_t* buffer, uint32_t* size ) {
#ifndef BYTE_ORDER
	#error Endianess not defined
#elif BYTE_ORDER == LITTLE_ENDIAN
	union double_union {
		double value;
		uint8_t chars[8];
	};
	double_union temp;
	temp.value = in_value;
	buffer[0] = temp.chars[7];
	buffer[1] = temp.chars[6];
	buffer[2] = temp.chars[5];
	buffer[3] = temp.chars[4];
	buffer[4] = temp.chars[3];
	buffer[5] = temp.chars[2];
	buffer[6] = temp.chars[1];
	buffer[7] = temp.chars[0];
	*size += 8;
#elif BYTE_ORDER == BIG_ENDIAN
	memcpy(buffer, &in_value, sizeof(in_value));
	*size += sizeof(in_value);
#endif
}
