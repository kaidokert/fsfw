#include "../config/cdatapool/dataPoolInit.h"
#include <fsfw/unittest/config/cdatapool/dataPoolInit.h>
#include <fsfw/unittest/config/objects/Factory.h>
#include <fsfw/unittest/core/CatchDefinitions.h>

#ifdef GCOV
#include <gcov.h>
#endif

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/datapool/DataPool.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>


/* Global instantiations normally done in main.cpp */
/* Initialize Data Pool */
//namespace glob {
DataPool dataPool(datapool::dataPoolInit);
//}


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

