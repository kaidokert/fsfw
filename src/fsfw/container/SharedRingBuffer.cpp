#include "fsfw/container/SharedRingBuffer.h"

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, const size_t size, bool overwriteOld,
                                   size_t maxExcessBytes)
    : SystemObject(objectId), SimpleRingBuffer(size, overwriteOld, maxExcessBytes) {
  mutex = MutexFactory::instance()->createMutex();
}

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, uint8_t* buffer, const size_t size,
                                   bool overwriteOld, size_t maxExcessBytes)
    : SystemObject(objectId), SimpleRingBuffer(buffer, size, overwriteOld, maxExcessBytes) {
  mutex = MutexFactory::instance()->createMutex();
}

SharedRingBuffer::~SharedRingBuffer() { MutexFactory::instance()->deleteMutex(mutex); }

void SharedRingBuffer::setToUseReceiveSizeFIFO(size_t fifoDepth) { this->fifoDepth = fifoDepth; }

ReturnValue_t SharedRingBuffer::lockRingBufferMutex(MutexIF::TimeoutType timeoutType,
                                                    dur_millis_t timeout) {
  return mutex->lockMutex(timeoutType, timeout);
}

ReturnValue_t SharedRingBuffer::unlockRingBufferMutex() { return mutex->unlockMutex(); }

MutexIF* SharedRingBuffer::getMutexHandle() const { return mutex; }

ReturnValue_t SharedRingBuffer::initialize() {
  if (fifoDepth > 0) {
    receiveSizesFIFO = new DynamicFIFO<size_t>(fifoDepth);
  }
  return SystemObject::initialize();
}

DynamicFIFO<size_t>* SharedRingBuffer::getReceiveSizesFIFO() {
  if (receiveSizesFIFO == nullptr) {
    // Configuration error.
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "SharedRingBuffer::getReceiveSizesFIFO: Ring buffer"
                 << " was not configured to have sizes FIFO, returning nullptr!" << std::endl;
#endif
  }
  return receiveSizesFIFO;
}
