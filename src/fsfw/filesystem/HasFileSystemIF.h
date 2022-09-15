#ifndef FSFW_MEMORY_HASFILESYSTEMIF_H_
#define FSFW_MEMORY_HASFILESYSTEMIF_H_

#include <cstddef>

#include "FileSystemArgsIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/returnvalues/returnvalue.h"

struct FilesystemParams {
  explicit FilesystemParams(const char* path) : path(path) {}

  const char* path;
  FileSystemArgsIF* args = nullptr;
};

struct FileOpParams {
  FileOpParams(const char* path, size_t size) : fsParams(path), size(size) {}

  [[nodiscard]] const char* path() const { return fsParams.path; }

  [[nodiscard]] FileSystemArgsIF* args() const { return fsParams.args; }

  FilesystemParams fsParams;
  size_t size;
  size_t offset = 0;
};

/**
 * @brief	Generic interface for objects which expose a file system to enable
 * 			message based file handling.
 * @author  J. Meier, R. Mueller
 */
class HasFileSystemIF {
 public:
  static constexpr uint8_t INTERFACE_ID = CLASS_ID::FILE_SYSTEM;

  //! [EXPORT] : P1: Can be file system specific error code
  static constexpr ReturnValue_t GENERIC_FILE_ERROR = MAKE_RETURN_CODE(0);
  static constexpr ReturnValue_t GENERIC_DIR_ERROR = MAKE_RETURN_CODE(1);
  static constexpr ReturnValue_t GENERIC_RENAME_ERROR = MAKE_RETURN_CODE(3);

  //! [EXPORT] : File system is currently busy
  static constexpr ReturnValue_t IS_BUSY = MAKE_RETURN_CODE(4);
  //! [EXPORT] : Invalid parameters like file name or repository path
  static constexpr ReturnValue_t INVALID_PARAMETERS = MAKE_RETURN_CODE(5);

  static constexpr ReturnValue_t FILE_DOES_NOT_EXIST = MAKE_RETURN_CODE(10);
  static constexpr ReturnValue_t FILE_ALREADY_EXISTS = MAKE_RETURN_CODE(11);
  static constexpr ReturnValue_t NOT_A_FILE = MAKE_RETURN_CODE(12);
  static constexpr ReturnValue_t FILE_LOCKED = MAKE_RETURN_CODE(13);
  static constexpr ReturnValue_t PERMISSION_DENIED = MAKE_RETURN_CODE(14);

  static constexpr ReturnValue_t DIRECTORY_DOES_NOT_EXIST = MAKE_RETURN_CODE(21);
  static constexpr ReturnValue_t DIRECTORY_ALREADY_EXISTS = MAKE_RETURN_CODE(22);
  static constexpr ReturnValue_t NOT_A_DIRECTORY = MAKE_RETURN_CODE(23);
  static constexpr ReturnValue_t DIRECTORY_NOT_EMPTY = MAKE_RETURN_CODE(24);

  //! [EXPORT] : P1: Sequence number missing
  static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_WRITE = MAKE_RETURN_CODE(30);
  //! [EXPORT] : P1: Sequence number missing
  static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_READ = MAKE_RETURN_CODE(31);

  virtual ~HasFileSystemIF() = default;

  /**
   * Function to get the MessageQueueId_t of the implementing object
   * @return MessageQueueId_t of the object
   */
  [[nodiscard]] virtual MessageQueueId_t getCommandQueue() const {
    return MessageQueueIF::NO_QUEUE;
  }

  virtual bool fileExists(FilesystemParams params) = 0;

  /**
   * Truncate a file, deleting its contents and setting its size to 0 accordingly.
   * @param params
   * @return
   */
  virtual ReturnValue_t truncateFile(FilesystemParams params) = 0;

  /**
   * @brief   Generic function to write to a file.
   *
   * @details
   * Implementations should not truncate the file. This is equivalent to opening a  file with "r+"
   * on Unix systems or using ios::out | ios::in with the C++ API.
   * @param fileOpInfo General information: File name, size to write, offset, additional arguments
   * @param data The data to write to the file
   */
  virtual ReturnValue_t writeToFile(FileOpParams params, const uint8_t* data) = 0;

  /**
   * @brief Generic function to read from a file. This variant takes a pointer to a buffer and
   * performs pointer arithmetic by incrementing the pointer by the read size
   * @param fileOpInfo General information: File name, size to write, offset, additional arguments
   * @param buffer [in/out] Data will be read into the provided buffer, and the pointer will be
   *    incremented by the read length
   * @param readSize [out] Will be incremented by the read length
   * @param maxSize Maximum size of the provided buffer
   * @param args
   * @return
   */
  virtual ReturnValue_t readFromFile(FileOpParams fileOpInfo, uint8_t** buffer, size_t& readSize,
                                     size_t maxSize) = 0;
  /**
   * Variant of the @readFromFile which does not perform pointer arithmetic.
   * @param fileOpInfo General information: File name, size to write, offset, additional arguments
   * @param buf
   * @param maxSize
   * @return
   */
  virtual ReturnValue_t readFromFile(FileOpParams fileOpInfo, uint8_t* buf, size_t maxSize) {
    size_t dummy = 0;
    return readFromFile(fileOpInfo, &buf, dummy, maxSize);
  }

  /**
   * @brief   Generic function to create a new file.
   * @param repositoryPath
   * @param filename
   * @param data
   * @param size
   * @param args Any other arguments which an implementation might require
   * @return
   */
  virtual ReturnValue_t createFile(FilesystemParams params) {
    return createFile(params, nullptr, 0);
  }
  virtual ReturnValue_t createFile(FilesystemParams params, const uint8_t* data, size_t size) = 0;

  /**
   * @brief   Generic function to delete a file.
   * @param repositoryPath
   * @param filename
   * @param args Any other arguments which an implementation might require
   * @return
   */
  virtual ReturnValue_t removeFile(const char* path, FileSystemArgsIF* args) = 0;
  virtual ReturnValue_t removeFile(const char* path) { return removeFile(path, nullptr); }

  /**
   * @brief   Generic function to create a directory
   * @param repositoryPath
   * @param   Equivalent to the -p flag in Unix systems. If some required parent directories
   *          do not exist, create them as well
   * @param args Any other arguments which an implementation might require
   * @return
   */
  virtual ReturnValue_t createDirectory(FilesystemParams params, bool createParentDirs) = 0;
  virtual ReturnValue_t createDirectory(FilesystemParams params) {
    return createDirectory(params, false);
  }

  /**
   * @brief   Generic function to remove a directory
   * @param repositoryPath
   * @param args Any other arguments which an implementation might require
   */
  virtual ReturnValue_t removeDirectory(FilesystemParams params, bool deleteRecurively) = 0;
  virtual ReturnValue_t removeDirectory(FilesystemParams params) {
    return removeDirectory(params, false);
  }

  virtual ReturnValue_t rename(const char* oldPath, const char* newPath) {
    return rename(oldPath, newPath, nullptr);
  }
  virtual ReturnValue_t rename(const char* oldPath, const char* newPath,
                               FileSystemArgsIF* args) = 0;
};

#endif /* FSFW_MEMORY_HASFILESYSTEMIF_H_ */
