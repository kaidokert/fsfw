#ifndef FSFW_PLATFORM_H_
#define FSFW_PLATFORM_H_

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_UNIX
#elif defined(_WIN32)
#define PLATFORM_WIN
#endif

#endif /* FSFW_PLATFORM_H_ */
