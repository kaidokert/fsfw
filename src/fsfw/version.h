#ifndef FSFW_SRC_FSFW_VERSION_H_
#define FSFW_SRC_FSFW_VERSION_H_

#include <cstdint>

namespace fsfw {

struct Version {
  uint32_t major = 0;
  uint32_t minor = 0;
  uint32_t revision = 0;
};

void getVersion(Version& version);

}

#endif /* FSFW_SRC_FSFW_VERSION_H_ */
