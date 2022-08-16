#ifndef LINUX_UTILITY_UNIXFILEGUARD_H_
#define LINUX_UTILITY_UNIXFILEGUARD_H_

#include <fcntl.h>
#include <fsfw/returnvalues/returnvalue.h>
#include <unistd.h>

#include <string>

class UnixFileGuard {
 public:
  static constexpr int READ_WRITE_FLAG = O_RDWR;
  static constexpr int READ_ONLY_FLAG = O_RDONLY;
  static constexpr int NON_BLOCKING_IO_FLAG = O_NONBLOCK;

  static constexpr ReturnValue_t OPEN_FILE_FAILED = 1;

  UnixFileGuard(std::string device, int* fileDescriptor, int flags,
                std::string diagnosticPrefix = "");

  virtual ~UnixFileGuard();

  ReturnValue_t getOpenResult() const;

 private:
  int* fileDescriptor = nullptr;
  ReturnValue_t openStatus = returnvalue::OK;
};

#endif /* LINUX_UTILITY_UNIXFILEGUARD_H_ */
