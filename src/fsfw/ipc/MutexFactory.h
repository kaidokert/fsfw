#ifndef FSFW_IPC_MUTEXFACTORY_H_
#define FSFW_IPC_MUTEXFACTORY_H_

#include "MutexIF.h"
/**
 * Creates Mutex.
 * This class is a "singleton" interface, i.e. it provides an
 * interface, but also is the base class for a singleton.
 */
class MutexFactory {
 public:
  virtual ~MutexFactory();
  /**
   * Returns the single instance of MutexFactory.
   * The implementation of #instance is found in its subclasses.
   * Thus, we choose link-time variability of the  instance.
   */
  static MutexFactory* instance();

  MutexIF* createMutex();

  void deleteMutex(MutexIF* mutex);

 private:
  /**
   * External instantiation is not allowed.
   */
  MutexFactory();
  static MutexFactory* factoryInstance;
};

#endif /* FSFW_IPC_MUTEXFACTORY_H_ */
