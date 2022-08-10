#include "FilesystemMock.h"

ReturnValue_t FilesystemMock::writeToFile(FileOpParams params, const uint8_t *data) {
  std::string filename(params.path);
  auto iter = fileMap.find(filename);
  if (iter == fileMap.end()) {
    FileSegmentQueue queue;
    queue.push(FileWriteInfo(filename, params.offset, data, params.size));
    FileInfo info;
    info.fileSegQueue = queue;
    info.fileRaw.insert(info.fileRaw.end(), data, data + params.size);
    fileMap.emplace(filename, info);
  } else {
    FileInfo& info = iter->second;
    info.fileSegQueue.push(FileWriteInfo(filename, params.offset, data, params.size));
    // Easiest case: append data to the end
    if(params.offset == info.fileRaw.size()) {
      info.fileRaw.insert(info.fileRaw.end(), data, data + params.size);
    } else {
      size_t totalNewLen = params.offset + params.size;
      if(totalNewLen > info.fileRaw.size()) {
        info.fileRaw.resize(params.offset + params.size);
      }
      std::copy(data, data + params.size, info.fileRaw.begin() + static_cast<unsigned int>(params.offset));
    }
  }
  return HasReturnvaluesIF::RETURN_OK;
}
ReturnValue_t FilesystemMock::readFromFile(FileOpParams fileOpInfo, uint8_t **buffer,
                                           size_t &readSize, size_t maxSize) {
  return 0;
}
ReturnValue_t FilesystemMock::createFile(FilesystemParams params, const uint8_t *data,
                                         size_t size) {
  return 0;
}
ReturnValue_t FilesystemMock::removeFile(const char *path, FileSystemArgsIF *args) { return 0; }
ReturnValue_t FilesystemMock::createDirectory(FilesystemParams params, bool createParentDirs) {
  return 0;
}
ReturnValue_t FilesystemMock::removeDirectory(FilesystemParams params, bool deleteRecurively) {
  return 0;
}
ReturnValue_t FilesystemMock::renameFile(const char *oldPath, char *newPath,
                                         FileSystemArgsIF *args) {
  return 0;
}
