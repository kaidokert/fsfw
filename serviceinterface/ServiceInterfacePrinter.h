#if FSFW_DISABLE_PRINTOUT == 0
#include <stdio.h>
#endif

namespace fsfw {

enum class PrintLevel {
	NONE = 0,
	//! Strange error when using just ERROR..
	ERROR_TYPE = 1,
	WARNING = 2,
	INFO = 3,
	DEBUG = 4
};

void setPrintLevel(PrintLevel printLevel);
PrintLevel getPrintLevel();

/**
 * These functions can be used like the C stdio printf and forward the
 * supplied formatted string arguments to a printf function.
 * They prepend the string with a color (if enabled), a log preamble and
 * a timestamp.
 * @param fmt Formatted string
 */
void printInfo(const char *fmt, ...);
void printWarning(const char* fmt, ...);
void printDebug(const char* fmt, ...);
void printError(const char* fmt, ...);

}

