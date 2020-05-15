#ifndef FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_
#define FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_
#include <cstdint>
#include <cstddef>

namespace printer {

enum class OutputType {
	DEC,
	HEX
};

void print(uint8_t* data, size_t size, OutputType type = OutputType::HEX);
void printHex(uint8_t* data, size_t size);
void printDec(uint8_t* data, size_t size);
}

#endif /* FRAMEWORK_GLOBALFUNCTIONS_PRINTER_H_ */
