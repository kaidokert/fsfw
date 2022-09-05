#ifndef FSFW_MOCKS_FILESYSTEMMOCK_H
#define FSFW_MOCKS_FILESYSTEMMOCK_H

#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <utility>

#include "fsfw/filesystem.h"

/**
 * This mock models a filesystem in the RAM. It can be used to verify correct behaviour of
 * a component using a filesystem without relying on an actual OS filesystem implementation.
 *
 * Please note that this object does not actually check paths for validity. The file API was
 * built in a way to allow reading a file back after it was written while also remembering
 * the specific file segments which were inserted in write calls.
 */
class FilesystemMock : public HasFileSystemIF {
 public:
  struct FileWriteInfo {
    FileWriteInfo(std::string filename, size_t offset, const uint8_t *data, size_t len)
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

  struct DirInfo {
    size_t createCallCount = 0;
    size_t delCallCount = 0;
    std::queue<bool> wihParentDir;
    std::queue<bool> recursiveDeletion;
  };
  std::map<std::string, DirInfo> dirMap;

  struct RenameInfo {
    RenameInfo(std::string oldName, std::string newName)
        : oldName(std::move(oldName)), newName(std::move(newName)) {}

    std::string oldName;
    std::string newName;
  };
  std::queue<RenameInfo> renameQueue;
  std::string truncateCalledOnFile;
  ReturnValue_t feedFile(const std::string &filename, std::ifstream &file);

  bool fileExists(FilesystemParams params) override;
  ReturnValue_t truncateFile(FilesystemParams params) override;

  ReturnValue_t writeToFile(FileOpParams params, const uint8_t *data) override;
  ReturnValue_t readFromFile(FileOpParams params, uint8_t **buffer, size_t &readSize,
                             size_t maxSize) override;
  ReturnValue_t createFile(FilesystemParams params, const uint8_t *data, size_t size) override;
  ReturnValue_t removeFile(const char *path, FileSystemArgsIF *args) override;
  ReturnValue_t createDirectory(FilesystemParams params, bool createParentDirs) override;
  ReturnValue_t removeDirectory(FilesystemParams params, bool deleteRecurively) override;
  ReturnValue_t rename(const char *oldPath, const char *newPath, FileSystemArgsIF *args) override;

  void reset();

  using HasFileSystemIF::createDirectory;
  using HasFileSystemIF::createFile;
  using HasFileSystemIF::readFromFile;

 private:
  void createOrAddToFile(FileOpParams params, const uint8_t *data);
};

#endif  // FSFW_MOCKS_FILESYSTEMMOCK_H
