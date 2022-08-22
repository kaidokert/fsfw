#ifndef FSFW_CONTAINER_FIFOBASE_H_
#define FSFW_CONTAINER_FIFOBASE_H_

#include <cstddef>
#include <cstring>

#include "../returnvalues/returnvalue.h"

template <typename T>
class FIFOBase {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::FIFO_CLASS;
  static const ReturnValue_t FULL = MAKE_RETURN_CODE(1);
  static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(2);

  /** Default ctor, takes pointer to first entry of underlying container
   *  and maximum capacity */
  FIFOBase(T* values, size_t maxCapacity);

  /**
   * Insert value into FIFO
   * @param value
   * @return returnvalue::OK on success, FULL if full
   */
  ReturnValue_t insert(T value);
  /**
   * Retrieve item from FIFO. This removes the item from the FIFO.
   * @param value Must point to a valid T
   * @return returnvalue::OK on success, EMPTY if empty and FAILED if nullptr check failed
   */
  ReturnValue_t retrieve(T* value);
  /**
   * Retrieve item from FIFO without removing it from FIFO.
   * @param value Must point to a valid T
   * @return returnvalue::OK on success, EMPTY if empty and FAILED if nullptr check failed
   */
  ReturnValue_t peek(T* value);
  /**
   * Remove item from FIFO.
   * @return returnvalue::OK on success, EMPTY if empty
   */
  ReturnValue_t pop();

  /***
   * Check if FIFO is empty
   * @return True if empty, False if not
   */
  bool empty();
  /***
   * Check if FIFO is Full
   * @return True if full, False if not
   */
  bool full();
  /***
   *  Current used size (elements) used
   * @return size_t in elements
   */
  size_t size();
  /***
   * Get maximal capacity of fifo
   * @return size_t with max capacity of this fifo
   */
  [[nodiscard]] size_t getMaxCapacity() const;

 protected:
  void setContainer(T* data);
  size_t maxCapacity = 0;

  T* values;

  size_t readIndex = 0;
  size_t writeIndex = 0;
  size_t currentSize = 0;

  size_t next(size_t current);
};

#include "FIFOBase.tpp"

#endif /* FSFW_CONTAINER_FIFOBASE_H_ */
