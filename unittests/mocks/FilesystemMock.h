#ifndef FSFW_MOCKS_FILESYSTEMMOCK_H
#define FSFW_MOCKS_FILESYSTEMMOCK_H

#include "fsfw/filesystem.h"

class FilesystemMock : public HasFileSystemIF {
 public:
  MessageQueueId_t getCommandQueue() const override;
  ReturnValue_t writeToFile(FileOpParams params, const uint8_t *data) override;
  ReturnValue_t readFromFile(FileOpParams fileOpInfo, uint8_t **buffer, size_t &readSize,
                             size_t maxSize) override;
  ReturnValue_t createFile(FilesystemParams params, const uint8_t *data, size_t size) override;
  ReturnValue_t removeFile(const char *path, FileSystemArgsIF *args) override;
  ReturnValue_t createDirectory(FilesystemParams params, bool createParentDirs) override;
  ReturnValue_t removeDirectory(FilesystemParams params, bool deleteRecurively) override;
  ReturnValue_t renameFile(const char *oldPath, char *newPath, FileSystemArgsIF *args) override;
};

#endif  // FSFW_MOCKS_FILESYSTEMMOCK_H
