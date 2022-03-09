#ifndef FSFW_SRC_FSFW_VERSION_H_
#define FSFW_SRC_FSFW_VERSION_H_

#include <cstdint>

namespace fsfw {

class Version {
 public:
  Version(uint32_t major, uint32_t minor, uint32_t revision);
  uint32_t major = 0;
  uint32_t minor = 0;
  uint32_t revision = 0;

  friend bool operator==(const Version& v1, const Version& v2) {
    return (v1.major == v2.major and v1.minor == v2.minor and v1.revision == v2.revision);
  }

  friend bool operator!=(const Version& v1, const Version& v2) { return not(v1 == v2); }

  friend bool operator<(const Version& v1, const Version& v2) {
    return ((v1.major < v2.major) or (v1.major == v2.major and v1.minor < v2.minor) or
            (v1.major == v2.major and v1.minor == v2.minor and v1.revision < v2.revision));
  }

  friend bool operator>(const Version& v1, const Version& v2) { return not(v1 < v2); }

  friend bool operator<=(const Version& v1, const Version& v2) { return ((v1 == v2) or (v1 < v2)); }

  friend bool operator>=(const Version& v1, const Version& v2) { return ((v1 == v2) or (v1 > v2)); }
};

extern const fsfw::Version FSFW_VERSION;

}  // namespace fsfw

#endif /* FSFW_SRC_FSFW_VERSION_H_ */
