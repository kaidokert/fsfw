#include "CatchDefinitions.h"
#include "CatchFactory.h"

#ifdef GCOV
#include <gcov.h>
#endif

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

/* Global instantiations normally done in main.cpp */
/* Initialize Data Pool */

int customSetup() {
  // global setup
  sif::initialize();
  ObjectManager* objMan = ObjectManager::instance();
  objMan->setObjectFactoryFunction(Factory::produceFrameworkObjects, nullptr);
  objMan->initialize();
  return 0;
}
