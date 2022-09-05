#ifndef FSFW_CFDP_STRINGLV_H
#define FSFW_CFDP_STRINGLV_H

#include <string>

#include "Lv.h"

namespace cfdp {

class StringLv : public Lv {
 public:
  StringLv();
  explicit StringLv(const std::string& fileName);
  explicit StringLv(const char* filename, size_t len);

  // Delete the move constructor to avoid passing in a temporary
  StringLv(const std::string&&) = delete;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_STRINGLV_H
