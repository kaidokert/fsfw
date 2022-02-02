#include "fsfw_hal/linux/UnixFileGuard.h"

#include <cerrno>
#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"

UnixFileGuard::UnixFileGuard(std::string device, int* fileDescriptor, int flags,
                             std::string diagnosticPrefix)
    : fileDescriptor(fileDescriptor) {
  if (fileDescriptor == nullptr) {
    return;
  }
  *fileDescriptor = open(device.c_str(), flags);
  if (*fileDescriptor < 0) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << diagnosticPrefix << ": Opening device failed with error code " << errno << ": "
                 << strerror(errno) << std::endl;
#else
    sif::printWarning("%s: Opening device failed with error code %d: %s\n", diagnosticPrefix, errno,
                      strerror(errno));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    openStatus = OPEN_FILE_FAILED;
  }
}

UnixFileGuard::~UnixFileGuard() {
  if (fileDescriptor != nullptr) {
    close(*fileDescriptor);
  }
}

ReturnValue_t UnixFileGuard::getOpenResult() const { return openStatus; }
