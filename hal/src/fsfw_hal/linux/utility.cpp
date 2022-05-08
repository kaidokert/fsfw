#include "fsfw_hal/linux/utility.h"

#include <cerrno>
#include <cstring>

#include "fsfw/serviceinterface.h"

void utility::handleIoctlError(const char* const customPrintout) {
  if (customPrintout != nullptr) {
    FSFW_LOGW(customPrintout);
  }
  FSFW_LOGW("Error code {} | {}\n", errno, strerror(errno));
}
