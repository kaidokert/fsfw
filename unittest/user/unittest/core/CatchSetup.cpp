#include "CatchFactory.h"
#include "CatchDefinitions.h"

#ifdef GCOV
#include <gcov.h>
#endif

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>


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

/* Global object manager */
ObjectManagerIF *objectManager;

int customSetup() {
    // global setup
    objectManager = new ObjectManager(Factory::produce);
    objectManager -> initialize();
    return 0;
}

