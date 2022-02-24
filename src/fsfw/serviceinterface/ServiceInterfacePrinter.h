#ifndef FSFW_SERVICEINTERFACE_SERVICEINTERFACEPRINTER
#define FSFW_SERVICEINTERFACE_SERVICEINTERFACEPRINTER

#include "fsfw/FSFW.h"

#if FSFW_DISABLE_PRINTOUT == 0
#include <cstdio>
#endif

//! https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
//! Can be used to print out binary numbers in human-readable format.
//! Example usage:
//! sif::printInfo("Status register: " BYTE_TO_BINARY_PATTERN "\n",BYTE_TO_BINARY(0x1f));
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                           \
  (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'), (byte & 0x20 ? '1' : '0'),     \
      (byte & 0x10 ? '1' : '0'), (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), \
      (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')

namespace sif {

enum PrintLevel {
  NONE = 0,
  //! Strange error when using just ERROR..
  ERROR_LEVEL = 1,
  WARNING_LEVEL = 2,
  INFO_LEVEL = 3,
  DEBUG_LEVEL = 4
};

/**
 * Set the print level. All print types with a smaller level will be printed
 * as well. For example, set to PrintLevel::WARNING to only enable error
 * and warning output.
 * @param printLevel
 */
void setPrintLevel(PrintLevel printLevel);
PrintLevel getPrintLevel();

void setToAddCrAtEnd(bool addCrAtEnd_);

/**
 * These functions can be used like the C stdio printf and forward the
 * supplied formatted string arguments to a printf function.
 * They prepend the string with a color (if enabled), a log preamble and
 * a timestamp.
 * @param fmt Formatted string
 */
void printInfo(const char* fmt, ...);
void printWarning(const char* fmt, ...);
void printDebug(const char* fmt, ...);
void printError(const char* fmt, ...);

}  // namespace sif

#endif /* FSFW_SERVICEINTERFACE_SERVICEINTERFACEPRINTER */
