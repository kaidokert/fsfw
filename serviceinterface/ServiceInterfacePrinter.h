#include <stdio.h>

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


void printInfo(const char *fmt, ...);
void printWarning(const char* fmt, ...);
void printDebug(const char* fmt, ...);
void printError(const char* fmt, ...);

}

