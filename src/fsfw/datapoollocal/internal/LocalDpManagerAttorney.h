#ifndef FSFW_DATAPOOLLOCAL_LOCALDPMANAGERATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_LOCALDPMANAGERATTORNEY_H_

#include "../LocalDataPoolManager.h"

/**
 * @brief	This is a helper class implements the Attorney-Client idiom for access to
 * 			LocalDataPoolManager internals
 * @details
 * This helper class provides better control over which classes are allowed to access
 * LocalDataPoolManager internals in a granular and encapsulated way when compared to
 * other methods like direct friend declarations. It allows these classes to use
 * an explicit subset of the pool manager private/protected functions.
 * See: https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Friendship_and_the_Attorney-Client
 */
class LocalDpManagerAttorney {
 private:
  template <typename T>
  static ReturnValue_t fetchPoolEntry(LocalDataPoolManager& manager, lp_id_t localPoolId,
                                      PoolEntry<T>** poolEntry) {
    return manager.fetchPoolEntry(localPoolId, poolEntry);
  }

  static MutexIF* getMutexHandle(LocalDataPoolManager& manager) { return manager.getMutexHandle(); }

  template <typename T>
  friend class LocalPoolVariable;
  template <typename T, uint16_t vecSize>
  friend class LocalPoolVector;
};

#endif /* FSFW_DATAPOOLLOCAL_LOCALDPMANAGERATTORNEY_H_ */
