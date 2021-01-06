#include <FSFWConfig.h>
#include "ServiceInterfacePrinter.h"
#include "serviceInterfaceDefintions.h"
#include "../timemanager/Clock.h"

#include <cstdarg>
#include <cstdint>

fsfw::PrintLevel printLevel = fsfw::PrintLevel::DEBUG;
#if defined(WIN32) && FSFW_COLORED_OUTPUT == 1
bool consoleInitialized = false;
#endif /* defined(WIN32) && FSFW_COLORED_OUTPUT == 1 */


#if FSFW_DISABLE_PRINTOUT == 0
uint8_t printBuffer[fsfwconfig::FSFW_PRINT_BUFFER_SIZE];

void fsfwPrint(fsfw::PrintLevel printType, const char* fmt, va_list arg) {

#if defined(WIN32) && FSFW_COLORED_OUTPUT == 1
	if(not consoleInitialized) {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
	consoleInitialized = true;
#endif

    size_t len = 0;
    char* bufferPosition = reinterpret_cast<char*>(printBuffer);

    /* Check logger level */
    if(printType == fsfw::PrintLevel::NONE or printType > printLevel) {
        return;
    }

    /* Log message to terminal */

#if FSFW_COLORED_OUTPUT == 1
    if(printType == fsfw::PrintLevel::INFO) {
    	len += sprintf(bufferPosition, fsfw::ANSI_COLOR_GREEN);
    }
    else if(printType == fsfw::PrintLevel::DEBUG) {
    	len += sprintf(bufferPosition, fsfw::ANSI_COLOR_MAGENTA);
    }
    else if(printType == fsfw::PrintLevel::WARNING) {
    	len += sprintf(bufferPosition, fsfw::ANSI_COLOR_YELLOW);
    }
    else if(printType == fsfw::PrintLevel::ERROR_TYPE) {
    	len += sprintf(bufferPosition, fsfw::ANSI_COLOR_RED);
    }
#endif

   if (printType == fsfw::PrintLevel::INFO) {
       len += sprintf(bufferPosition + len, "INFO: ");
   }
   if(printType == fsfw::PrintLevel::DEBUG) {
	   len += sprintf(bufferPosition + len, "DEBUG: ");
   }
   if(printType == fsfw::PrintLevel::WARNING) {
	   len += sprintf(bufferPosition + len, "WARNING: ");
   }

   if(printType == fsfw::PrintLevel::ERROR_TYPE) {
	   len += sprintf(bufferPosition + len, "ERROR: ");
   }

    Clock::TimeOfDay_t now;
    Clock::getDateAndTime(&now);
    /*
     * Log current time to terminal if desired.
     */
    len += sprintf(bufferPosition + len, "| %lu:%02lu:%02lu.%03lu | ",
    		(unsigned long) now.hour,
			(unsigned long) now.minute,
			(unsigned long) now.second,
			(unsigned long) now.usecond /1000);

    len += vsnprintf(bufferPosition + len, sizeof(printBuffer) - len, fmt, arg);

    printf("%s", printBuffer);
}


void fsfw::printInfo(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fsfwPrint(fsfw::PrintLevel::INFO, fmt, args);
    va_end(args);
}

void fsfw::printWarning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fsfwPrint(fsfw::PrintLevel::WARNING, fmt, args);
    va_end(args);
}

void fsfw::printDebug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fsfwPrint(fsfw::PrintLevel::DEBUG, fmt, args);
    va_end(args);
}

void fsfw::printError(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fsfwPrint(fsfw::PrintLevel::ERROR_TYPE, fmt, args);
    va_end(args);
}

#else

void fsfw::printInfo(const char *fmt, ...) {}
void fsfw::printWarning(const char *fmt, ...) {}
void fsfw::printDebug(const char *fmt, ...) {}
void fsfw::printError(const char *fmt, ...) {}

#endif /* FSFW_DISABLE_PRINTOUT == 0 */

void fsfw::setPrintLevel(PrintLevel printLevel_) {
	printLevel = printLevel_;
}

fsfw::PrintLevel fsfw::getPrintLevel() {
	return printLevel;
}
