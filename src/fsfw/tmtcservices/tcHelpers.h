#ifndef FSFW_TMTCSERVICES_TCHELPERS_H
#define FSFW_TMTCSERVICES_TCHELPERS_H

#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tmtcpacket/pus/tc.h"

namespace tc {

ReturnValue_t prepareTcReader(StorageManagerIF& tcStore, store_address_t storeId,
                              PusTcReader& tcReader);

}  // namespace tc

#endif  // FSFW_TMTCSERVICES_TCHELPERS_H
