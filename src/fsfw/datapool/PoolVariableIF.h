#ifndef FSFW_DATAPOOL_POOLVARIABLEIF_H_
#define FSFW_DATAPOOL_POOLVARIABLEIF_H_

#include "../returnvalues/returnvalue.h"
#include "../serialize/SerializeIF.h"
#include "ReadCommitIF.h"

/**
 * @brief	This interface is used to control data pool
 * 			variable representations.
 * @details
 * To securely handle data pool variables, all pool entries are locally
 * managed by data pool variable access classes, which are called pool
 * variables. To ensure a common state of a set of variables needed in a
 * function, these local pool variables again are managed by other classes,
 * like the DataSet classes. This interface provides unified access to
 * local pool variables for such manager classes.
 * @author 	Bastian Baetz
 * @ingroup data_pool
 */
class PoolVariableIF : public SerializeIF, public ReadCommitIF {
 public:
  static constexpr uint8_t INTERFACE_ID = CLASS_ID::POOL_VARIABLE_IF;
  static constexpr ReturnValue_t INVALID_READ_WRITE_MODE = MAKE_RETURN_CODE(0xA0);
  static constexpr ReturnValue_t INVALID_POOL_ENTRY = MAKE_RETURN_CODE(0xA1);

  static constexpr bool VALID = 1;
  static constexpr bool INVALID = 0;
  static constexpr uint32_t NO_PARAMETER = 0xffffffff;

  enum ReadWriteMode_t { VAR_READ, VAR_WRITE, VAR_READ_WRITE };

  /**
   * @brief	This is an empty virtual destructor,
   * 			as it is proposed for C++ interfaces.
   */
  virtual ~PoolVariableIF() {}
  /**
   * @brief	This method returns if the variable is write-only,
   * 			read-write or read-only.
   */
  virtual ReadWriteMode_t getReadWriteMode() const = 0;
  virtual void setReadWriteMode(ReadWriteMode_t newMode) = 0;

  /**
   * @brief	This operation shall return the data pool id of the variable.
   */
  virtual uint32_t getDataPoolId() const = 0;
  /**
   * @brief	With this call, the valid information of the
   * 			variable is returned.
   */
  virtual bool isValid() const = 0;
  /**
   * @brief	With this call, the valid information of the variable is set.
   */
  virtual void setValid(bool validity) = 0;
};

using pool_rwm_t = PoolVariableIF::ReadWriteMode_t;

#endif /* FSFW_DATAPOOL_POOLVARIABLEIF_H_ */
