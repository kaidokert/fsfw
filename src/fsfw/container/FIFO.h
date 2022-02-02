#ifndef FSFW_CONTAINER_FIFO_H_
#define FSFW_CONTAINER_FIFO_H_

#include <array>

#include "FIFOBase.h"

/**
 * @brief     Simple First-In-First-Out data structure with size fixed at
 *             compile time
 * @details
 * Performs no dynamic memory allocation.
 * The public interface of FIFOBase exposes the user interface for the FIFO.
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template <typename T, size_t capacity>
class FIFO : public FIFOBase<T> {
 public:
  FIFO() : FIFOBase<T>(nullptr, capacity) { this->setContainer(fifoArray.data()); };

  /**
   * @brief     Custom copy constructor to set pointer correctly.
   * @param other
   */
  FIFO(const FIFO& other) : FIFOBase<T>(other) {
    this->fifoArray = other.fifoArray;
    this->setContainer(fifoArray.data());
  }

  /**
   * @brief Custom assignment operator
   * @param other
   */
  FIFO& operator=(const FIFO& other) {
    FIFOBase<T>::operator=(other);
    this->fifoArray = other.fifoArray;
    this->setContainer(fifoArray.data());
    return *this;
  }

 private:
  std::array<T, capacity> fifoArray;
};

#endif /* FSFW_CONTAINER_FIFO_H_ */
