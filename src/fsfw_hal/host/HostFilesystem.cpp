#include "HostFilesystem.h"

#include <filesystem>
#include <fstream>

#include "fsfw/serialize.h"

using namespace std::filesystem;
using namespace std;

HostFilesystem::HostFilesystem() = default;

ReturnValue_t HostFilesystem::writeToFile(FileOpParams params, const uint8_t *data) {
  if (params.path() == nullptr) {
    return returnvalue::FAILED;
  }
  path path(params.path());
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  // This is equivalent to "r+" mode, which is what we need here. Only using ::out would truncate
  // the file
  ofstream file(path, ios::binary | ios::out | ios::in);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  file.seekp(static_cast<unsigned int>(params.offset));
  file.write(reinterpret_cast<const char *>(data), static_cast<unsigned int>(params.size));
  return returnvalue::OK;
}

ReturnValue_t HostFilesystem::readFromFile(FileOpParams params, uint8_t **buffer, size_t &readSize,
                                           size_t maxSize) {
  if (params.path() == nullptr) {
    return returnvalue::FAILED;
  }
  path path(params.path());
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  ifstream file(path);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  auto sizeToRead = static_cast<unsigned int>(params.size);
  file.seekg(static_cast<unsigned int>(params.offset));
  if (readSize + sizeToRead > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  file.read(reinterpret_cast<char *>(*buffer), sizeToRead);
  readSize += sizeToRead;
  *buffer += sizeToRead;
  return returnvalue::OK;
}

ReturnValue_t HostFilesystem::createFile(FilesystemParams params, const uint8_t *data,
                                         size_t size) {
  if (params.path == nullptr) {
    return returnvalue::FAILED;
  }
  path path(params.path);
  if (exists(path)) {
    return HasFileSystemIF::FILE_ALREADY_EXISTS;
  }
  ofstream file(path);
  if (file.fail()) {
    return HasFileSystemIF::GENERIC_FILE_ERROR;
  }
  return returnvalue::OK;
}

ReturnValue_t HostFilesystem::removeFile(const char *path_, FileSystemArgsIF *args) {
  if (path_ == nullptr) {
    return returnvalue::FAILED;
  }
  path path(path_);
  if (not exists(path)) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  }
  if (remove(path, errorCode)) {
    return returnvalue::OK;
  }
  return HasFileSystemIF::GENERIC_FILE_ERROR;
}

ReturnValue_t HostFilesystem::createDirectory(FilesystemParams params, bool createParentDirs) {
  if (params.path == nullptr) {
    return returnvalue::FAILED;
  }
  path dirPath(params.path);

  if (exists(dirPath)) {
    return HasFileSystemIF::DIRECTORY_ALREADY_EXISTS;
  }

  if (createParentDirs) {
    if (create_directories(dirPath, errorCode)) {
      return returnvalue::OK;
    }
    return HasFileSystemIF::GENERIC_DIR_ERROR;
  }
  if (create_directory(dirPath, errorCode)) {
    return returnvalue::OK;
  }
  return HasFileSystemIF::GENERIC_DIR_ERROR;
}

ReturnValue_t HostFilesystem::removeDirectory(FilesystemParams params, bool deleteRecurively) {
  if (params.path == nullptr) {
    return returnvalue::FAILED;
  }
  path dirPath(params.path);
  if (not exists(dirPath)) {
    return HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST;
  }
  if (is_regular_file(dirPath)) {
    return HasFileSystemIF::NOT_A_DIRECTORY;
  }
  if (deleteRecurively) {
    if (remove_all(dirPath, errorCode)) {
      return returnvalue::OK;
    }
  } else {
    if (remove(dirPath, errorCode)) {
      return returnvalue::OK;
    }
  }
  // Error handling
  if (errorCode == std::errc::directory_not_empty) {
    return HasFileSystemIF::DIRECTORY_NOT_EMPTY;
  }
  return HasFileSystemIF::GENERIC_DIR_ERROR;
}

ReturnValue_t HostFilesystem::rename(const char *oldPath_, const char *newPath_,
                                     FileSystemArgsIF *args) {
  if (oldPath_ == nullptr or newPath_ == nullptr) {
    return returnvalue::FAILED;
  }
  path oldPath(oldPath_);
  path newPath(newPath_);
  errorCode.clear();
  std::filesystem::rename(oldPath, newPath, errorCode);
  if (errorCode) {
    return HasFileSystemIF::GENERIC_RENAME_ERROR;
  }
  return returnvalue::OK;
}

bool HostFilesystem::fileExists(FilesystemParams params) {
  path path(params.path);
  return filesystem::exists(path);
}

ReturnValue_t HostFilesystem::truncateFile(FilesystemParams params) {
  path path(params.path);
  if (not filesystem::exists(path)) {
    return FILE_DOES_NOT_EXIST;
  }
  ofstream of(path);
  return returnvalue::OK;
}
