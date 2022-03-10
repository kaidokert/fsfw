#include "version.h"

#include "fsfw/FSFWVersion.h"

#undef major
#undef minor

const fsfw::Version fsfw::FSFW_VERSION = {FSFW_VERSION_MAJOR, FSFW_VERSION_MINOR,
                                          FSFW_VERSION_REVISION};

fsfw::Version::Version(uint32_t major, uint32_t minor, uint32_t revision)
    : major(major), minor(minor), revision(revision) {}
