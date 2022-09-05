#include "FilesystemMock.h"

#include <filesystem>

#include "fsfw/serialize/SerializeIF.h"

ReturnValue_t FilesystemMock::feedFile(const std::string &filename, std::ifstream &file) {
  if (not std::filesystem::exists(filename)) {
    return returnvalue::FAILED;
  }
  size_t fileSize = std::filesystem::file_size(filename);
  FileOpParams params(filename.c_str(), fileSize);
  std::vector<uint8_t> rawData(fileSize);
  file.read(reinterpret_cast<char *>(rawData.data()), static_cast<unsigned int>(rawData.size()));
  createOrAddToFile(params, rawData.data());
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::writeToFile(FileOpParams params, const uint8_t *data) {
  createOrAddToFile(params, data);
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::readFromFile(FileOpParams params, uint8_t **buffer, size_t &readSize,
                                           size_t maxSize) {
  std::string filename(params.path());
  auto iter = fileMap.find(filename);
  if (iter == fileMap.end()) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  } else {
    FileInfo &info = iter->second;
    size_t readLen = params.size;
    if (params.offset + params.size > info.fileRaw.size()) {
      if (params.offset > info.fileRaw.size()) {
        return returnvalue::OK;
      }
      readLen = info.fileRaw.size() - params.offset;
    }
    if (readSize + readLen > maxSize) {
      return SerializeIF::STREAM_TOO_SHORT;
    }
    std::copy(info.fileRaw.data() + params.offset, info.fileRaw.data() + readLen, *buffer);
    *buffer += readLen;
    readSize += readLen;
  }
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::createFile(FilesystemParams params, const uint8_t *data,
                                         size_t size) {
  FileOpParams params2(params.path, size);
  createOrAddToFile(params2, data);
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::removeFile(const char *path, FileSystemArgsIF *args) {
  std::string filename(path);
  auto iter = fileMap.find(filename);
  if (iter == fileMap.end()) {
    return HasFileSystemIF::FILE_DOES_NOT_EXIST;
  } else {
    fileMap.erase(iter);
    return returnvalue::OK;
  }
}

ReturnValue_t FilesystemMock::createDirectory(FilesystemParams params, bool createParentDirs) {
  std::string dirPath = params.path;
  dirMap[dirPath].createCallCount++;
  dirMap[dirPath].wihParentDir.push(createParentDirs);
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::removeDirectory(FilesystemParams params, bool deleteRecurively) {
  std::string dirPath = params.path;
  dirMap[dirPath].delCallCount++;
  dirMap[dirPath].recursiveDeletion.push(deleteRecurively);
  return returnvalue::OK;
}

ReturnValue_t FilesystemMock::rename(const char *oldPath, const char *newPath,
                                     FileSystemArgsIF *args) {
  renameQueue.push(RenameInfo(oldPath, newPath));
  return returnvalue::OK;
}

void FilesystemMock::createOrAddToFile(FileOpParams params, const uint8_t *data) {
  std::string filename(params.path());
  auto iter = fileMap.find(filename);
  if (iter == fileMap.end()) {
    FileSegmentQueue queue;
    if (params.size > 0) {
      queue.push(FileWriteInfo(filename, params.offset, data, params.size));
    }
    FileInfo info;
    info.fileSegQueue = queue;
    if (data != nullptr) {
      info.fileRaw.insert(info.fileRaw.end(), data, data + params.size);
    }
    fileMap.emplace(filename, info);
  } else {
    FileInfo &info = iter->second;
    info.fileSegQueue.push(FileWriteInfo(filename, params.offset, data, params.size));
    if (data == nullptr) {
      return;
    }
    // Easiest case: append data to the end
    if (params.offset == info.fileRaw.size()) {
      info.fileRaw.insert(info.fileRaw.end(), data, data + params.size);
    } else {
      size_t totalNewLen = params.offset + params.size;
      if (totalNewLen > info.fileRaw.size()) {
        info.fileRaw.resize(params.offset + params.size);
      }
      std::copy(data, data + params.size,
                info.fileRaw.begin() + static_cast<unsigned int>(params.offset));
    }
  }
}

void FilesystemMock::reset() {
  fileMap.clear();
  dirMap.clear();
  std::queue<RenameInfo> empty;
  std::swap(renameQueue, empty);
}

bool FilesystemMock::fileExists(FilesystemParams params) {
  std::string filename(params.path);
  auto iter = fileMap.find(filename);
  if (iter == fileMap.end()) {
    return false;
  }
  return true;
}

ReturnValue_t FilesystemMock::truncateFile(FilesystemParams params) {
  truncateCalledOnFile = params.path;
  return returnvalue::OK;
}
