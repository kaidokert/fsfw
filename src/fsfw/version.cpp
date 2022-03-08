#include "version.h"
#include "fsfw/FSFWVersion.h"

void fsfw::getVersion(Version& v) {
  v.major = FSFW_VERSION;
  v.minor = FSFW_SUBVERSION;
  v.revision = FSFW_REVISION;
}
