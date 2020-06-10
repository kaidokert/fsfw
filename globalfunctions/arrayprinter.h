#ifndef FRAMEWORK_GLOBALFUNCTIONS_ARRAYPRINTER_H_
#define FRAMEWORK_GLOBALFUNCTIONS_ARRAYPRINTER_H_
#include <cstdint>
#include <cstddef>

enum class OutputType {
	DEC,
	HEX,
	BIN
};

namespace arrayprinter {
void print(const uint8_t* data, size_t size, OutputType type = OutputType::HEX,
		bool printInfo = true, size_t maxCharPerLine = 12);
void printHex(const uint8_t* data, size_t size, size_t maxCharPerLine = 12);
void printDec(const uint8_t* data, size_t size, size_t maxCharPerLine = 12);
void printBin(const uint8_t* data, size_t size);
}

#endif /* FRAMEWORK_GLOBALFUNCTIONS_ARRAYPRINTER_H_ */
