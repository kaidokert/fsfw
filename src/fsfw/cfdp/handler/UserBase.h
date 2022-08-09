#ifndef FSFW_CFDP_USERBASE_H
#define FSFW_CFDP_USERBASE_H

#include "fsfw/memory/HasFileSystemIF.h"

namespace cfdp {

class UserBase {
 public:
  explicit UserBase(HasFileSystemIF& vfs);

 private:
  HasFileSystemIF& vfs;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_USERBASE_H
