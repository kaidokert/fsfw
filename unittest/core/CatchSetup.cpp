#include "CatchDefinitions.h"
#include <CatchFactory.h>

#include <testcfg/cdatapool/dataPoolInit.h>

#ifdef GCOV
#include <gcov.h>
#endif

#include "../../objectmanager/ObjectManager.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "../../storagemanager/StorageManagerIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"


/* Global instantiations normally done in main.cpp */
/* Initialize Data Pool */


namespace sif {
/* Set up output streams */
ServiceInterfaceStream debug("DEBUG");
ServiceInterfaceStream info("INFO");
ServiceInterfaceStream error("ERROR");
ServiceInterfaceStream warning("WARNING");
}

/* Global object manager */
ObjectManagerIF *objectManager;

int customSetup() {
    // global setup
    objectManager = new ObjectManager(Factory::produce);
    objectManager -> initialize();
    return 0;
}

