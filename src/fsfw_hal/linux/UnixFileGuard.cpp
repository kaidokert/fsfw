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
    FSFW_LOGW("{} | Opening device failed with error code {} | {}\n", diagnosticPrefix, errno,
              strerror(errno));
    openStatus = OPEN_FILE_FAILED;
  }
}

UnixFileGuard::~UnixFileGuard() {
  if (fileDescriptor != nullptr) {
    close(*fileDescriptor);
  }
}

ReturnValue_t UnixFileGuard::getOpenResult() const { return openStatus; }
