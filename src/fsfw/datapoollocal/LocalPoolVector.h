#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_H_

#include "../datapool/DataSetIF.h"
#include "../datapool/PoolEntry.h"
#include "../datapool/PoolVariableIF.h"
#include "../datapoollocal/LocalDataPoolManager.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterface.h"
#include "LocalPoolObjectBase.h"
#include "internal/LocalDpManagerAttorney.h"

/**
 * @brief	This is the access class for array-type data pool entries.
 * @details
 * To ensure safe usage of the data pool, operation is not done directly on the
 * data pool entries, but on local copies. This class provides simple type-
 * and length-safe access to vector-style data pool entries (i.e. entries with
 * length > 1). The class can be instantiated as read-write and read only.
 *
 * It provides a commit-and-roll-back semantic, which means that no array
 * entry in the data pool is changed until the commit call is executed.
 * There are two template parameters:
 * @tparam	T
 * This template parameter specifies the data type of an array entry. Currently,
 * all plain data types are supported, but in principle any type is possible.
 * @tparam  vector_size
 * This template parameter specifies the vector size of this entry. Using a
 * template parameter for this is not perfect, but avoids
 * dynamic memory allocation.
 * @ingroup data_pool
 */
template <typename T, uint16_t vectorSize>
class LocalPoolVector : public LocalPoolObjectBase {
 public:
  LocalPoolVector() = delete;
  /**
   * This constructor is used by the data creators to have pool variable
   * instances which can also be stored in datasets.
   * It does not fetch the current value from the data pool. This is performed
   * by the read() operation (which is not thread-safe).
   * Datasets can be used to access local pool entires in a thread-safe way.
   * @param poolId ID of the local pool entry.
   * @param hkOwner Pointer of the owner. This will generally be the calling
   * class itself which passes "this".
   * @param setReadWriteMode Specify the read-write mode of the pool variable.
   * @param dataSet The data set in which the variable shall register itself.
   * If nullptr, the variable is not registered.
   */
  LocalPoolVector(HasLocalDataPoolIF* hkOwner, lp_id_t poolId, DataSetIF* dataSet = nullptr,
                  pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);

  /**
   * This constructor is used by data users like controllers to have
   * access to the local pool variables of data creators by supplying
   * the respective creator object ID.
   * It does not fetch the current value from the data pool. This is performed
   * by the read() operation (which is not thread-safe).
   * Datasets can be used to access local pool entires in a thread-safe way.
   * @param poolId ID of the local pool entry.
   * @param hkOwner Pointer of the owner. This will generally be the calling
   * class itself which passes "this".
   * @param setReadWriteMode Specify the read-write mode of the pool variable.
   * @param dataSet The data set in which the variable shall register itself.
   * If nullptr, the variable is not registered.
   */
  LocalPoolVector(object_id_t poolOwner, lp_id_t poolId, DataSetIF* dataSet = nullptr,
                  pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);
  /**
   * Variation which takes the unique global identifier of a local pool
   * vector.
   * @param globalPoolId
   * @param dataSet
   * @param setReadWriteMode
   */
  LocalPoolVector(gp_id_t globalPoolId, DataSetIF* dataSet = nullptr,
                  pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);

  /**
   * @brief	This is the local copy of the data pool entry.
   * @details
   * The user can work on this attribute just like he would on a local
   * array of this type.
   */
  T value[vectorSize] = {};
  /**
   * @brief	The classes destructor is empty.
   * @details	If commit() was not called, the local value is
   * 			discarded and not written back to the data pool.
   */
  ~LocalPoolVector(){};
  /**
   * @brief	The operation returns the number of array entries
   * 			in this variable.
   */
  uint8_t getSize() { return vectorSize; }

  T& operator[](size_t i);
  const T& operator[](size_t i) const;

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, const size_t maxSize,
                                  SerializeIF::Endianness streamEndiannes) const override;
  virtual size_t getSerializedSize() const override;
  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    SerializeIF::Endianness streamEndianness) override;

  /**
   * @brief	This is a call to read the array's values
   * 			from the global data pool.
   * @details
   * When executed, this operation tries to fetch the pool entry with matching
   * data pool id from the data pool and copies all array values and the valid
   * information to its local attributes.
   * In case of a failure (wrong type, size or pool id not found), the
   * variable is set to zero and invalid.
   * The read call is protected with a lock.
   * It is recommended to use DataSets to read and commit multiple variables
   * at once to avoid the overhead of unnecessary lock und unlock operations.
   */
  ReturnValue_t read(MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                     uint32_t timeoutMs = 20) override;

  /**
   * @brief	The commit call copies the array values back to the data pool.
   * @details
   * It checks type and size, as well as if the variable is writable. If so,
   * the value is copied and the local valid flag is written back as well.
   * The read call is protected with a lock.
   * It is recommended to use DataSets to read and commit multiple variables
   * at once to avoid the overhead of unnecessary lock und unlock operations.
   */
  ReturnValue_t commit(MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                       uint32_t timeoutMs = 20) override;

  /**
   * @brief	This commit call also sets the validity of the pool entry.
   * @details
   */
  ReturnValue_t commit(bool valid, MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                       uint32_t timeoutMs = 20);

 protected:
  /**
   * @brief	Like #read, but without a lock protection of the global pool.
   * @details
   * The operation does NOT provide any mutual exclusive protection by itself.
   * This can be used if the lock is handled externally to avoid the overhead
   * of consecutive lock und unlock operations.
   * Declared protected to discourage free public usage.
   */
  ReturnValue_t readWithoutLock() override;
  /**
   * @brief	Like #commit, but without a lock protection of the global pool.
   * @details
   * The operation does NOT provide any mutual exclusive protection by itself.
   * This can be used if the lock is handled externally to avoid the overhead
   * of consecutive lock und unlock operations.
   * Declared protected to discourage free public usage.
   */
  ReturnValue_t commitWithoutLock() override;

 private:
#if FSFW_CPP_OSTREAM_ENABLED == 1
  // std::ostream is the type for object std::cout
  template <typename U, uint16_t otherSize>
  friend std::ostream& operator<<(std::ostream& out, const LocalPoolVector<U, otherSize>& var);
#endif
};

#include "LocalPoolVector.tpp"

template <typename T, uint16_t vectorSize>
using lp_vec_t = LocalPoolVector<T, vectorSize>;

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_H_ */
