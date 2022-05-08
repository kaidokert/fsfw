#ifndef FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_
#define FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_

#include "fsfw/FSFW.h"
#include "serviceInterfaceDefintions.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include "ServiceInterfaceStream.h"
#else
#include "ServiceInterfacePrinter.h"
#endif

#endif /* FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_ */
