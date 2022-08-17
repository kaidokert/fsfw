#ifndef FSFW_CONTAINER_DYNAMICFIFO_H_
#define FSFW_CONTAINER_DYNAMICFIFO_H_

#include <vector>

#include "FIFOBase.h"

/**
 * @brief     Simple First-In-First-Out data structure. The maximum size
 *             can be set in the constructor.
 * @details
 * The maximum capacity can be determined at run-time, so this container
 * performs dynamic memory allocation!
 * The public interface of FIFOBase exposes the user interface for the FIFO.
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template <typename T>
class DynamicFIFO : public FIFOBase<T> {
 public:
  explicit DynamicFIFO(size_t maxCapacity)
      : FIFOBase<T>(nullptr, maxCapacity), fifoVector(maxCapacity) {
    // trying to pass the pointer of the uninitialized vector
    // to the FIFOBase constructor directly lead to a super evil bug.
    // So we do it like this now.
    this->setContainer(fifoVector.data());
  };

  /**
   * @brief    Custom copy constructor which prevents setting the
   *             underlying pointer wrong. This function allocates memory!
   * @details This is a very heavy operation so try to avoid this!
   *
   */
  DynamicFIFO(const DynamicFIFO& other) : FIFOBase<T>(other), fifoVector(other.maxCapacity) {
    this->fifoVector = other.fifoVector;
    this->setContainer(fifoVector.data());
  }

  /**
   * @brief Custom assignment operator
   * @details This is a very heavy operation so try to avoid this!
   * @param other DyamicFIFO to copy from
   */
  DynamicFIFO& operator=(const DynamicFIFO& other) {
    FIFOBase<T>::operator=(other);
    this->fifoVector = other.fifoVector;
    this->setContainer(fifoVector.data());
    return *this;
  }

 private:
  std::vector<T> fifoVector;
};

#endif /* FSFW_CONTAINER_DYNAMICFIFO_H_ */
