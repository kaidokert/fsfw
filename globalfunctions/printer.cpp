#include <framework/globalfunctions/printer.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

void printer::print(uint8_t *data, size_t size, OutputType type) {
	sif::info << "StorageAccessor: Printing data with size " << size << ": [";
	if(type == OutputType::HEX) {
		printer::printHex(data, size);
	}
	else {
		printer::printDec(data, size);
	}
}

void printer::printHex(uint8_t *data, size_t size) {
	sif::info << std::hex;
	for(size_t i = 0; i < size; i++) {
		sif::info << "0x" << static_cast<int>(data[i]);
		if(i < size - 1){
			sif::info << " , ";
		}
	}
	sif::info << std::dec;
	sif::info << " ] " << std::endl;
}

void printer::printDec(uint8_t *data, size_t size) {
	for(size_t i = 0; i < size; i++) {
		sif::info << "0x" << static_cast<int>(data[i]);
		if(i < size - 1){
			sif::info << " , ";
		}
	}
}

