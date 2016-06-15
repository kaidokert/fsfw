/*
 * conversion.h
 *
 *  Created on: 07.05.2012
 *      Author: baetz
 */

#ifndef CONVERSION_H_
#define CONVERSION_H_


#include <framework/osal/OSAL.h>


void convertToByteStream( uint16_t value, uint8_t* buffer, uint32_t* size );

void convertToByteStream( uint32_t value, uint8_t* buffer, uint32_t* size );

void convertToByteStream( int16_t value, uint8_t* buffer, uint32_t* size );

void convertToByteStream( int32_t value, uint8_t* buffer, uint32_t* size );

//void convertToByteStream( uint64_t value, uint8_t* buffer, uint32_t* size );
//
//void convertToByteStream( int64_t value, uint8_t* buffer, uint32_t* size );

void convertToByteStream( float value, uint8_t* buffer, uint32_t* size );

void convertToByteStream( double value, uint8_t* buffer, uint32_t* size );

#endif /* CONVERSION_H_ */
