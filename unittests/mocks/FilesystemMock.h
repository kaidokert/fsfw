#ifndef FSFW_MOCKS_FILESYSTEMMOCK_H
#define FSFW_MOCKS_FILESYSTEMMOCK_H

#include <queue>
#include <string>
#include <map>
#include <utility>

#include "fsfw/filesystem.h"

class FilesystemMock : public HasFileSystemIF {
 public:
  struct FileWriteInfo {
    FileWriteInfo(std::string filename, size_t offset, const uint8_t* data, size_t len)
      : filename(std::move(filename)), offset(offset) {
      this->data.insert(this->data.end(), data, data + len);
    }
    std::string filename;
    size_t offset;
    std::vector<uint8_t> data;
  };
  using FileSegmentQueue = std::queue<FileWriteInfo>;

  struct FileInfo {
    FileSegmentQueue fileSegQueue;
    std::vector<uint8_t> fileRaw;
  };

  std::map<std::string, FileInfo> fileMap;

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
