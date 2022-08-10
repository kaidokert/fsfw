#include "HostFilesystem.h"

#include <filesystem>
#include <fstream>

#include "fsfw/serialize.h"

using namespace std::filesystem;
using namespace std;

HostFilesystem::HostFilesystem() = default;

ReturnValue_t HostFilesystem::writeToFile(FileOpParams params, const uint8_t *data) {
  if (params.path == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  path path(params.path);
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  ofstream file(path, ios::binary | ios::out);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  file.seekp(static_cast<unsigned int>(params.offset));
  file.write(reinterpret_cast<const char *>(data), static_cast<unsigned int>(params.size));
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HostFilesystem::readFromFile(FileOpParams params, uint8_t **buffer, size_t &readSize,
                                           size_t maxSize) {
  if (params.path == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  path path(params.path);
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  ifstream file(path);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  auto readLen = static_cast<unsigned int>(params.offset);
  file.seekg(readLen);
  if (readSize + params.size > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  file.read(reinterpret_cast<char *>(*buffer), readLen);
  readSize += readLen;
  *buffer += readLen;
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HostFilesystem::createFile(FilesystemParams params, const uint8_t *data,
                                         size_t size) {
  if (params.path == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  path path(params.path);
  if (exists(path)) {
    return HasFileSystemIF::FILE_ALREADY_EXISTS;
  }
  ofstream file(path);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HostFilesystem::removeFile(const char *path_, FileSystemArgsIF *args) {
  if (path_ == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  path path(path_);
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  if (remove(path, errorCode)) {
    return HasReturnvaluesIF::RETURN_OK;
  }
  return HasFileSystemIF::GENERIC_FILE_ERROR;
}
ReturnValue_t HostFilesystem::createDirectory(FilesystemParams params, bool createParentDirs) {
  if (params.path == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  path dirPath(params.path);
  if (createParentDirs) {
    create_directories(dirPath, errorCode);
  }
  return 0;
}
ReturnValue_t HostFilesystem::removeDirectory(FilesystemParams params, bool deleteRecurively) {
  return 0;
}
ReturnValue_t HostFilesystem::renameFile(const char *oldPath, char *newPath,
                                         FileSystemArgsIF *args) {
  return 0;
}
