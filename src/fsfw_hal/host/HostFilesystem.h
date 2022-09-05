#ifndef FSFW_HAL_HOSTFILESYSTEM_H
#define FSFW_HAL_HOSTFILESYSTEM_H

#include <system_error>

#include "fsfw/filesystem/HasFileSystemIF.h"

class HostFilesystem : public HasFileSystemIF {
 public:
  HostFilesystem();

  bool fileExists(FilesystemParams params) override;
  ReturnValue_t truncateFile(FilesystemParams params) override;
  ReturnValue_t writeToFile(FileOpParams params, const uint8_t *data) override;
  ReturnValue_t readFromFile(FileOpParams fileOpInfo, uint8_t **buffer, size_t &readSize,
                             size_t maxSize) override;
  ReturnValue_t createFile(FilesystemParams params, const uint8_t *data, size_t size) override;
  ReturnValue_t removeFile(const char *path, FileSystemArgsIF *args) override;
  ReturnValue_t createDirectory(FilesystemParams params, bool createParentDirs) override;
  ReturnValue_t removeDirectory(FilesystemParams params, bool deleteRecurively) override;
  ReturnValue_t rename(const char *oldPath, const char *newPath, FileSystemArgsIF *args) override;

  std::error_code errorCode;
  using HasFileSystemIF::createDirectory;
  using HasFileSystemIF::createFile;
  using HasFileSystemIF::removeDirectory;
  using HasFileSystemIF::removeFile;
  using HasFileSystemIF::rename;

 private:
};
#endif  // FSFW_HAL_HOSTFILESYSTEM_H
