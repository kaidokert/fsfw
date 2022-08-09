#ifndef FSFW_CFDP_USERBASE_H
#define FSFW_CFDP_USERBASE_H

#include "fsfw/memory/HasFileSystemIF.h"

namespace cfdp {

class UserBase {
 public:
  /**
   * Create a user base class which is used to provides a user interface to interact with CFDP
   * handlers. It is also used to pass the Virtual Filestore (VFS) Implementation to the CFDP
   * handlers so the filestore operations can be mapped to the underlying filestore.
   * @param vfs Virtual Filestore Object. Will be used for all file operations
   */
  explicit UserBase(HasFileSystemIF& vfs);

 private:
  HasFileSystemIF& vfs;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_USERBASE_H
