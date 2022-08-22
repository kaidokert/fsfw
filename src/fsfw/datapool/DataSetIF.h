#ifndef FSFW_DATAPOOL_DATASETIF_H_
#define FSFW_DATAPOOL_DATASETIF_H_

#include "../returnvalues/returnvalue.h"
#include "../timemanager/Clock.h"
class PoolVariableIF;

/**
 * @brief	This class defines a small interface to register on a DataSet.
 *
 * @details
 * Currently, the only purpose of this interface is to provide a
 * method for locally checked-out variables to register on a data set.
 * Still, it may become useful for other purposes as well.
 * @author 	Bastian Baetz
 * @ingroup data_pool
 */
class DataSetIF {
 public:
  static constexpr uint8_t INTERFACE_ID = CLASS_ID::DATA_SET_CLASS;
  static constexpr ReturnValue_t INVALID_PARAMETER_DEFINITION = MAKE_RETURN_CODE(1);
  static constexpr ReturnValue_t SET_WAS_ALREADY_READ = MAKE_RETURN_CODE(2);
  static constexpr ReturnValue_t COMMITING_WITHOUT_READING = MAKE_RETURN_CODE(3);

  static constexpr ReturnValue_t DATA_SET_UNINITIALISED = MAKE_RETURN_CODE(4);
  static constexpr ReturnValue_t DATA_SET_FULL = MAKE_RETURN_CODE(5);
  static constexpr ReturnValue_t POOL_VAR_NULL = MAKE_RETURN_CODE(6);

  /**
   * @brief	This is an empty virtual destructor,
   * 			as it is proposed for C++ interfaces.
   */
  virtual ~DataSetIF() {}

  /**
   * @brief	This operation provides a method to register local data pool
   * 			variables to register in a data set by passing itself
   * 			to this DataSet operation.
   */
  virtual ReturnValue_t registerVariable(PoolVariableIF* variable) = 0;

  virtual uint16_t getFillCount() const = 0;
};

#endif /* FSFW_DATAPOOL_DATASETIF_H_ */
