#ifndef FSFW_TASKS_SEMAPHOREFACTORY_H_
#define FSFW_TASKS_SEMAPHOREFACTORY_H_

#include "fsfw/tasks/SemaphoreIF.h"

/**
 * Creates Semaphore.
 * This class is a "singleton" interface, i.e. it provides an
 * interface, but also is the base class for a singleton.
 */
class SemaphoreFactory {
 public:
  virtual ~SemaphoreFactory();
  /**
   * Returns the single instance of SemaphoreFactory.
   * The implementation of #instance is found in its subclasses.
   * Thus, we choose link-time variability of the  instance.
   */
  static SemaphoreFactory* instance();

  /**
   * Create a binary semaphore.
   * Creator function for a binary semaphore which may only be acquired once
   * @param argument Can be used to pass implementation specific information.
   * @return Pointer to newly created semaphore class instance.
   */
  SemaphoreIF* createBinarySemaphore(uint32_t arguments = 0);
  /**
   * Create a counting semaphore.
   * Creator functons for a counting semaphore which may be acquired multiple
   * times.
   * @param count Semaphore can be taken count times.
   * @param initCount Initial count value.
   * @param argument Can be used to pass implementation specific information.
   * @return
   */
  SemaphoreIF* createCountingSemaphore(const uint8_t maxCount, uint8_t initCount,
                                       uint32_t arguments = 0);

  void deleteSemaphore(SemaphoreIF* semaphore);

 private:
  /**
   * External instantiation is not allowed.
   */
  SemaphoreFactory();
  static SemaphoreFactory* factoryInstance;
};

#endif /* FSFW_TASKS_SEMAPHOREFACTORY_H_ */
