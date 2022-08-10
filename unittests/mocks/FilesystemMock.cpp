#include "FilesystemMock.h"

ReturnValue_t FilesystemMock::writeToFile(FileOpParams params, const uint8_t *data) { return 0; }
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
