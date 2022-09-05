#ifndef FSFW_SRC_FSFW_MEMORY_FILESYSTEMARGS_H_
#define FSFW_SRC_FSFW_MEMORY_FILESYSTEMARGS_H_

/**
 * Empty base interface which can be implemented by to pass arguments via the HasFileSystemIF.
 * Users can then dynamic_cast the base pointer to the require child pointer.
 */
class FileSystemArgsIF {
 public:
  virtual ~FileSystemArgsIF() = default;
};

#endif /* FSFW_SRC_FSFW_MEMORY_FILESYSTEMARGS_H_ */
