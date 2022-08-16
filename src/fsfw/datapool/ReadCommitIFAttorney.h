#ifndef FSFW_DATAPOOL_READCOMMITIFATTORNEY_H_
#define FSFW_DATAPOOL_READCOMMITIFATTORNEY_H_

#include <fsfw/datapool/ReadCommitIF.h>
#include <fsfw/returnvalues/returnvalue.h>

/**
 * @brief   This class determines which members are allowed to access protected members
 *          of the ReadCommitIF.
 */
class ReadCommitIFAttorney {
 private:
  static ReturnValue_t readWithoutLock(ReadCommitIF* readCommitIF) {
    if (readCommitIF == nullptr) {
      return returnvalue::FAILED;
    }
    return readCommitIF->readWithoutLock();
  }

  static ReturnValue_t commitWithoutLock(ReadCommitIF* readCommitIF) {
    if (readCommitIF == nullptr) {
      return returnvalue::FAILED;
    }
    return readCommitIF->commitWithoutLock();
  }

  friend class PoolDataSetBase;
};

#endif /* FSFW_DATAPOOL_READCOMMITIFATTORNEY_H_ */
