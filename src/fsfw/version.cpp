#include "version.h"
#include "fsfw/FSFWVersion.h"

#include <cstdio>

#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

const fsfw::Version fsfw::FSFW_VERSION = {FSFW_VERSION_MAJOR, FSFW_VERSION_MINOR,
                                          FSFW_VERSION_REVISION};

fsfw::Version::Version(uint32_t major, uint32_t minor, uint32_t revision)
    : major(major), minor(minor), revision(revision) {}

void fsfw::Version::getVersion(char* str, size_t maxLen) const {
  snprintf(str, maxLen, "%d.%d.%d", major, minor, revision);
}
