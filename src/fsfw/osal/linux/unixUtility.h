#ifndef FSFW_OSAL_LINUX_UNIXUTILITY_H_
#define FSFW_OSAL_LINUX_UNIXUTILITY_H_

#include "../../serviceinterface/serviceInterfaceDefintions.h"

namespace utility {

void printUnixErrorGeneric(const char* const className, const char* const function,
                           const char* const failString,
                           sif::OutputTypes outputType = sif::OutputTypes::OUT_ERROR);

}

#endif /* FSFW_OSAL_LINUX_UNIXUTILITY_H_ */
