#include "fsfw/osal/linux/unixUtility.h"

#include <errno.h>

#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

void utility::printUnixErrorGeneric(const char* const className, const char* const function,
                                    const char* const failString, sif::OutputTypes outputType) {
  if (className == nullptr or failString == nullptr or function == nullptr) {
    return;
  }
#if FSFW_VERBOSE_LEVEL >= 1
  if (outputType == sif::OutputTypes::OUT_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << className << "::" << function << ": " << failString
               << " error: " << strerror(errno) << std::endl;
#else
    sif::printError("%s::%s: %s error: %s\n", className, function, failString, strerror(errno));
#endif
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << className << "::" << function << ": " << failString
                 << " error: " << strerror(errno) << std::endl;
#else
    sif::printWarning("%s::%s: %s error: %s\n", className, function, failString, strerror(errno));
#endif
  }
#endif
}
