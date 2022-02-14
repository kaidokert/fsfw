#ifndef FRAMEWORK_OSAL_ENDINESS_H_
#define FRAMEWORK_OSAL_ENDINESS_H_

/*
 * BSD-style endian declaration
 */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

// This is a GCC C extension
#ifndef BYTE_ORDER_SYSTEM
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define BYTE_ORDER_SYSTEM LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BYTE_ORDER_SYSTEM BIG_ENDIAN
#else
#error "Can't decide which end is which!"
#endif
#else

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#define BYTE_ORDER_SYSTEM LITTLE_ENDIAN
#else
#define BYTE_ORDER_SYSTEM BIG_ENDIAN
#endif

#else
#error __BYTE_ORDER__ not defined
#endif

#endif

#endif

#endif /* FRAMEWORK_OSAL_ENDINESS_H_ */
