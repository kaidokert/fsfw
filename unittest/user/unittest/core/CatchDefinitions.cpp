#include "CatchDefinitions.h"
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>

StorageManagerIF* tglob::getIpcStoreHandle() {
	if(objectManager != nullptr) {
		return objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	} else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Global object manager uninitialized" << std::endl;
#else
		sif::printError("Global object manager uninitialized\n\r");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
		return nullptr;
	}
}
