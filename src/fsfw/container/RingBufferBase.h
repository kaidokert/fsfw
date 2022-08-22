#ifndef FSFW_CONTAINER_RINGBUFFERBASE_H_
#define FSFW_CONTAINER_RINGBUFFERBASE_H_

#include <cstddef>

#include "../returnvalues/returnvalue.h"

template <uint8_t N_READ_PTRS = 1>
class RingBufferBase {
 public:
  RingBufferBase(size_t startAddress, const size_t size, bool overwriteOld)
      : start(startAddress), write(startAddress), size(size), overwriteOld(overwriteOld) {
    for (uint8_t count = 0; count < N_READ_PTRS; count++) {
      read[count] = startAddress;
    }
  }

  virtual ~RingBufferBase() {}

  bool isFull(uint8_t n = 0) { return (availableWriteSpace(n) == 0); }
  bool isEmpty(uint8_t n = 0) { return (getAvailableReadData(n) == 0); }

  size_t getAvailableReadData(uint8_t n = 0) const { return ((write + size) - read[n]) % size; }
  size_t availableWriteSpace(uint8_t n = 0) const {
    // One less to avoid ambiguous full/empty problem.
    return (((read[n] + size) - write - 1) % size);
  }

  bool overwritesOld() const { return overwriteOld; }

  size_t getMaxSize() const { return size - 1; }

  void clear() {
    write = start;
    for (uint8_t count = 0; count < N_READ_PTRS; count++) {
      read[count] = start;
    }
  }

  size_t writeTillWrap() { return (start + size) - write; }

  size_t readTillWrap(uint8_t n = 0) { return (start + size) - read[n]; }

  size_t getStart() const { return start; }

 protected:
  const size_t start;
  size_t write;
  size_t read[N_READ_PTRS];
  const size_t size;
  const bool overwriteOld;

  void incrementWrite(uint32_t amount) { write = ((write + amount - start) % size) + start; }
  void incrementRead(uint32_t amount, uint8_t n = 0) {
    read[n] = ((read[n] + amount - start) % size) + start;
  }

  ReturnValue_t readData(uint32_t amount, uint8_t n = 0) {
    if (getAvailableReadData(n) >= amount) {
      incrementRead(amount, n);
      return returnvalue::OK;
    } else {
      return returnvalue::FAILED;
    }
  }

  ReturnValue_t writeData(uint32_t amount) {
    if (availableWriteSpace() >= amount or overwriteOld) {
      incrementWrite(amount);
      return returnvalue::OK;
    } else {
      return returnvalue::FAILED;
    }
  }

  size_t getRead(uint8_t n = 0) const { return read[n]; }

  void setRead(uint32_t read, uint8_t n = 0) {
    if (read >= start && read < (start + size)) {
      this->read[n] = read;
    }
  }

  uint32_t getWrite() const { return write; }

  void setWrite(uint32_t write) { this->write = write; }
};

#endif /* FSFW_CONTAINER_RINGBUFFERBASE_H_ */
