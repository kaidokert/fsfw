#include "CatchDefinitions.h"

#include <fsfw/objectmanager/ObjectManager.h>

#include "fsfw/serviceinterface.h"

StorageManagerIF* tglob::getIpcStoreHandle() {
  if (ObjectManager::instance() != nullptr) {
    return ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Global object manager uninitialized" << std::endl;
#else
    FSFW_LOGE("Global object manager uninitialized\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
    return nullptr;
  }
}
