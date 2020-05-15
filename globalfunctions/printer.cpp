#include <framework/globalfunctions/printer.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

void printer::print(uint8_t *data, size_t size) {
	sif::info << "StorageAccessor: Printing data: [";
	for(size_t i = 0; i < size; i++) {
		sif::info << std::hex << (int)data[i];
		if(i < size - 1){
			sif::info << " , ";
		}
	}
	sif::info << " ] " << std::endl;
}
