#ifndef FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_
#define FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_

class LocalDataPoolManager;
class MutexIF;

/**
 * @brief 	Accessor class which can be used by classes which like to use the pool manager.
 */
class AccessPoolManagerIF {
 public:
  virtual ~AccessPoolManagerIF(){};

  virtual MutexIF* getLocalPoolMutex() = 0;

  /**
   * Can be used to get a handle to the local data pool manager.
   * This function is protected because it should only be used by the
   * class imlementing the interface.
   */
  virtual LocalDataPoolManager* getPoolManagerHandle() = 0;

 protected:
};

#endif /* FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_ */
