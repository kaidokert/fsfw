#ifndef FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_
#define FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_
#include <cstdint>
#include <cstddef>

namespace printer {

enum class OutputType {
	DEC,
	HEX
};

void print(const uint8_t* data, size_t size, OutputType type = OutputType::HEX,
		bool printInfo = true, size_t maxCharPerLine = 12);
void printHex(const uint8_t* data, size_t size, size_t maxCharPerLine = 12);
void printDec(const uint8_t* data, size_t size, size_t maxCharPerLine = 12);
}

#endif /* FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_ */
