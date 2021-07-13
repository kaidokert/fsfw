#ifndef FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_
#define FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_

#include <fsfw/ipc/messageQueueDefinitions.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>

namespace retval {
static constexpr int CATCH_OK = static_cast<int>(HasReturnvaluesIF::RETURN_OK);
static constexpr int CATCH_FAILED = static_cast<int>(HasReturnvaluesIF::RETURN_FAILED);
}

namespace tconst {
	static constexpr MessageQueueId_t testQueueId = 42;
}

namespace tglob {
	StorageManagerIF* getIpcStoreHandle();
}

#endif /* FSFW_UNITTEST_CORE_CATCHDEFINITIONS_H_ */
