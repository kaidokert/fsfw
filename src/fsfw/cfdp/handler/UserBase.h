#ifndef FSFW_CFDP_USERBASE_H
#define FSFW_CFDP_USERBASE_H

#include "fsfw/filesystem/HasFileSystemIF.h"

namespace cfdp {

class UserBase {
 public:
  /**
   * @brief Base class which provides a user interface to interact with CFDP handlers.
   *
   * @details
   * This class is also used to pass the Virtual Filestore (VFS) Implementation to the CFDP
   * handlers so the filestore operations can be mapped to the underlying filestore.
   *
   * It is used by implementing it in a child class and then passing it to the CFDP
   * handler objects. The base class provides default implementation for the user indication
   * primitives specified in the CFDP standard. The user can override these implementations
   * to provide custom indication handlers.
   * @param vfs Virtual Filestore Object. Will be used for all file operations
   */
  explicit UserBase(HasFileSystemIF& vfs);

 private:
  HasFileSystemIF& vfs;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_USERBASE_H
