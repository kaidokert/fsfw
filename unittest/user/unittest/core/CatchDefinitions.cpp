#include "CatchDefinitions.h"
#include <fsfw/objectmanager/ObjectManagerIF.h>

StorageManagerIF* tglob::getIpcStoreHandle() {
	if(objectManager != nullptr) {
		return objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	} else {
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "Global object manager uninitialized" << std::endl;
#endif
		return nullptr;
	}
}
