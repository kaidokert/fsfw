#include "arrayprinter.h"
#include "../serviceinterface/ServiceInterface.h"
#include <bitset>

void arrayprinter::print(const uint8_t *data, size_t size, OutputType type,
        bool printInfo, size_t maxCharPerLine) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    if(printInfo) {
        sif::info << "Printing data with size " << size << ": ";
    }
    sif::info << "[";
#else
#if FSFW_NO_C99_IO == 1
    sif::printInfo("Printing data with size %lu: [", static_cast<unsigned long>(size));
#else
    sif::printInfo("Printing data with size %zu: [", size);
#endif /* FSFW_NO_C99_IO == 1 */
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
    if(type == OutputType::HEX) {
        arrayprinter::printHex(data, size, maxCharPerLine);
    }
    else if (type == OutputType::DEC) {
        arrayprinter::printDec(data, size, maxCharPerLine);
    }
    else if(type == OutputType::BIN) {
        arrayprinter::printBin(data, size);
    }
}

void arrayprinter::printHex(const uint8_t *data, size_t size,
        size_t maxCharPerLine) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << std::hex;
    for(size_t i = 0; i < size; i++) {
        sif::info << "0x" << static_cast<int>(data[i]);
        if(i < size - 1) {
            sif::info << " , ";
            if(i > 0 and i % maxCharPerLine == 0) {
                sif::info << std::endl;

            }
        }
    }
    sif::info << std::dec;
    sif::info << "]" << std::endl;
#else
    // General format: 0x01, 0x02, 0x03 so it is number of chars times 6
    // plus line break plus small safety margin.
    char printBuffer[(size + 1) * 7 + 1];
    size_t currentPos = 0;
    for(size_t i = 0; i < size; i++) {
        // To avoid buffer overflows.
        if(sizeof(printBuffer) - currentPos <= 7) {
            break;
        }

        currentPos += snprintf(printBuffer + currentPos, 6, "0x%02x", data[i]);
        if(i < size - 1) {
            currentPos += sprintf(printBuffer + currentPos, ", ");
            if(i > 0 and i % maxCharPerLine == 0) {
                currentPos += sprintf(printBuffer + currentPos, "\n");
            }
        }
    }
#endif
}

void arrayprinter::printDec(const uint8_t *data, size_t size,
        size_t maxCharPerLine) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << std::dec;
    for(size_t i = 0; i < size; i++) {
        sif::info << static_cast<int>(data[i]);
        if(i < size - 1){
            sif::info << " , ";
            if(i > 0 and i % maxCharPerLine == 0) {
                sif::info << std::endl;
            }
        }
    }
    sif::info << "]" << std::endl;
#else
    // General format: 32, 243, -12 so it is number of chars times 5
    // plus line break plus small safety margin.
    char printBuffer[(size + 1) * 5 + 1];
    size_t currentPos = 0;
    for(size_t i = 0; i < size; i++) {
        // To avoid buffer overflows.
        if(sizeof(printBuffer) - currentPos <= 5) {
            break;
        }

        currentPos += snprintf(printBuffer + currentPos, 3, "%d", data[i]);
        if(i < size - 1) {
            currentPos += sprintf(printBuffer + currentPos, ", ");
            if(i > 0 and i % maxCharPerLine == 0) {
                currentPos += sprintf(printBuffer + currentPos, "\n");
            }
        }
    }
#endif
}

void arrayprinter::printBin(const uint8_t *data, size_t size) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "\n" << std::flush;
    for(size_t i = 0; i < size; i++) {
        sif::info << "Byte " << i + 1 << ": 0b" <<  std::bitset<8>(data[i]) << "," << std::endl;
    }
    sif::info << "]" << std::endl;
#else
    sif::printInfo("\n");
    for(size_t i = 0; i < size; i++) {
        sif::printInfo("Byte %d: 0b" BYTE_TO_BINARY_PATTERN ",\n", BYTE_TO_BINARY(data[i]));
    }
    sif::printInfo("]\n");
#endif
}
