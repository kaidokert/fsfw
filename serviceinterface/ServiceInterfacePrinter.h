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


static const char* const ANSI_COLOR_RED = "\x1b[31m";
static const char* const ANSI_COLOR_GREEN = "\x1b[32m";
static const char* const ANSI_COLOR_YELLOW = "\x1b[33m";
static const char* const ANSI_COLOR_BLUE = "\x1b[34m";
static const char* const ANSI_COLOR_MAGENTA = "\x1b[35m";
static const char* const ANSI_COLOR_CYAN = "\x1b[36m";
static const char* const ANSI_COLOR_RESET = "\x1b[0m";

void setPrintLevel(PrintLevel printLevel);
PrintLevel getPrintLevel();


void printInfo(const char *fmt, ...);
void printWarning(const char* fmt, ...);
void printDebug(const char* fmt, ...);
void printError(const char* fmt, ...);

}

