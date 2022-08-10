#include "UserBase.h"

#include <array>
#include <iostream>

cfdp::UserBase::UserBase(HasFileSystemIF& vfs) : vfs(vfs) {}
