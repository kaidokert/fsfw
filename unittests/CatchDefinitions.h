#ifndef FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_
#define FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_

#include <fsfw/ipc/messageQueueDefinitions.h>
#include <fsfw/returnvalues/returnvalue.h>
#include <fsfw/storagemanager/StorageManagerIF.h>

#include "fsfw/FSFW.h"

namespace tconst {
static constexpr MessageQueueId_t testQueueId = 42;
}

namespace tglob {
StorageManagerIF* getIpcStoreHandle();
}

#endif /* FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_ */
