#ifndef FRAMEWORK_TASKS_SEMAPHOREIF_H_
#define FRAMEWORK_TASKS_SEMAPHOREIF_H_
#include <cstdint>

#include "../returnvalues/FwClassIds.h"
#include "../returnvalues/returnvalue.h"

/**
 * @brief 	Generic interface for semaphores, which can be used to achieve
 * 			task synchronization. This is a generic interface which can be
 * 			used for both binary semaphores and counting semaphores.
 * @details
 * A semaphore is a synchronization primitive.
 * See: https://en.wikipedia.org/wiki/Semaphore_(programming)
 * A semaphore can be used to achieve task synchonization and track the
 * availability of resources by using either the binary or the counting
 * semaphore types.
 *
 * If mutual exlcusion of a resource is desired, a mutex should be used,
 * which is a special form of a semaphore and has an own interface.
 */
class SemaphoreIF {
 public:
  /**
   * Different types of timeout for the mutex lock.
   */
  enum TimeoutType {
    POLLING,  //!< If mutex is not available, return immediately
    WAITING,  //!< Wait a specified time for the mutex to become available
    BLOCKING  //!< Block indefinitely until the mutex becomes available.
  };

  virtual ~SemaphoreIF(){};

  static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;
  //! Semaphore timeout
  static constexpr ReturnValue_t SEMAPHORE_TIMEOUT = MAKE_RETURN_CODE(1);
  //! The current semaphore can not be given, because it is not owned
  static constexpr ReturnValue_t SEMAPHORE_NOT_OWNED = MAKE_RETURN_CODE(2);
  static constexpr ReturnValue_t SEMAPHORE_INVALID = MAKE_RETURN_CODE(3);

  /**
   * Generic call to acquire a semaphore.
   * If there are no more semaphores to be taken (for a counting semaphore,
   * a semaphore may be taken more than once), the taks will block
   * for a maximum of timeoutMs while trying to acquire the semaphore.
   * This can be used to achieve task synchrnization.
   * @param timeoutMs
   * @return - c returnvalue::OK for successfull acquisition
   */
  virtual ReturnValue_t acquire(TimeoutType timeoutType = TimeoutType::BLOCKING,
                                uint32_t timeoutMs = 0) = 0;

  /**
   * Corrensponding call to release a semaphore.
   * @return -@c returnvalue::OK for successfull release
   */
  virtual ReturnValue_t release() = 0;

  /**
   * If the semaphore is a counting semaphore then the semaphores current
   * count value is returned. If the semaphore is a binary semaphore then 1
   * is returned if the semaphore is available, and 0 is returned if the
   * semaphore is not available.
   */
  virtual uint8_t getSemaphoreCounter() const = 0;
};

#endif /* FRAMEWORK_TASKS_SEMAPHOREIF_H_ */
