#include "version.h"

#include <cstdio>

#include "fsfw/FSFWVersion.h"

#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

const fsfw::Version fsfw::FSFW_VERSION = {FSFW_VERSION_MAJOR, FSFW_VERSION_MINOR,
                                          FSFW_VERSION_REVISION, FSFW_VCS_INFO};

fsfw::Version::Version(int major, int minor, int revision, const char* addInfo)
    : major(major), minor(minor), revision(revision), addInfo(addInfo) {}

void fsfw::Version::getVersion(char* str, size_t maxLen) const {
  size_t len = snprintf(str, maxLen, "%d.%d.%d", major, minor, revision);
  if (addInfo != nullptr) {
    snprintf(str + len, maxLen - len, "-%s", addInfo);
  }
}

namespace fsfw {

#if FSFW_CPP_OSTREAM_ENABLED == 1
std::ostream& operator<<(std::ostream& os, const Version& v) {
  os << v.major << "." << v.minor << "." << v.revision;
  if (v.addInfo != nullptr) {
    os << "-" << v.addInfo;
  }
  return os;
}
#endif

}  // namespace fsfw
