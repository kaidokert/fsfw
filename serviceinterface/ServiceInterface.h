#ifndef FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_
#define FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_

#include <FSFWConfig.h>
#include "serviceInterfaceDefintions.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#else
#include <fsfw/serviceinterface/ServiceInterfacePrinter.h>
#endif

#endif /* FSFW_SERVICEINTERFACE_SERVICEINTERFACE_H_ */
