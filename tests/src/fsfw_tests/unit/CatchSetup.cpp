#include "CatchFactory.h"
#include "CatchDefinitions.h"

#ifdef GCOV
#include <gcov.h>
#endif

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"


/* Global instantiations normally done in main.cpp */
/* Initialize Data Pool */

#if FSFW_CPP_OSTREAM_ENABLED == 1
namespace sif {
/* Set up output streams */
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream error("ERROR");
ServiceInterfaceStream warning("WARNING");
}
#endif

int customSetup() {
    // global setup
    ObjectManager* objMan = ObjectManager::instance();
    objMan->setObjectFactoryFunction(Factory::produceFrameworkObjects, nullptr);
    objMan->initialize();
    return 0;
}

