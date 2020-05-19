#include <framework/globalfunctions/printer.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

void printer::print(const uint8_t *data, size_t size, OutputType type,
		bool printInfo, size_t maxCharPerLine) {
	if(printInfo) {
		sif::info << "Printing data with size " << size << ": ";
	}
	sif::info << "[";
	if(type == OutputType::HEX) {
		printer::printHex(data, size, maxCharPerLine);
	}
	else {
		printer::printDec(data, size, maxCharPerLine);
	}
}

void printer::printHex(const uint8_t *data, size_t size,
		size_t maxCharPerLine) {
	sif::info << std::hex;
	for(size_t i = 0; i < size; i++) {
		sif::info << "0x" << static_cast<int>(data[i]);
		if(i < size - 1){
			sif::info << " , ";
			if(i > 0 and i % maxCharPerLine == 0) {
				sif::info << std::endl;
			}
		}

	}
	sif::info << std::dec;
	sif::info << "]" << std::endl;
}

void printer::printDec(const uint8_t *data, size_t size,
		size_t maxCharPerLine) {
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
}
