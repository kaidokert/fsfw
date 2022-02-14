#include "fsfw_hal/linux/utility.h"

#include <cerrno>
#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

void utility::handleIoctlError(const char* const customPrintout) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  if (customPrintout != nullptr) {
    sif::warning << customPrintout << std::endl;
  }
  sif::warning << "handleIoctlError: Error code " << errno << ", " << strerror(errno) << std::endl;
#else
  if (customPrintout != nullptr) {
    sif::printWarning("%s\n", customPrintout);
  }
  sif::printWarning("handleIoctlError: Error code %d, %s\n", errno, strerror(errno));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
}
