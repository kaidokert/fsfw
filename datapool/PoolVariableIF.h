#ifndef POOLVARIABLEIF_H_
#define POOLVARIABLEIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/SerializeIF.h>


/**
 * @brief	This interface is used to control data pool variable representations.
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
class PoolVariableIF : public SerializeIF {
	friend class GlobDataSet;
	friend class LocalDataSet;
protected:
	/**
	 * @brief	The commit call shall write back a newly calculated local
	 * 			value to the data pool.
	 */
	virtual ReturnValue_t commit() = 0;
	/**
	 * @brief	The read call shall read the value of this parameter from
	 * 			the data pool and store the content locally.
	 */
	virtual ReturnValue_t read() = 0;
public:
	static constexpr bool VALID = 1;
	static constexpr bool INVALID = 0;
	static constexpr uint32_t NO_PARAMETER = 0;

	enum ReadWriteMode_t {
		VAR_READ, VAR_WRITE, VAR_READ_WRITE
	};

	/**
	 * @brief	This is an empty virtual destructor,
	 * 			as it is proposed for C++ interfaces.
	 */
	virtual ~PoolVariableIF() {}
	/**
	 * @brief	This method returns if the variable is write-only, read-write or read-only.
	 */
	virtual ReadWriteMode_t getReadWriteMode() const = 0;
	/**
	 * @brief	This operation shall return the data pool id of the variable.
	 */
	virtual uint32_t getDataPoolId() const = 0;
	/**
	 * @brief	With this call, the valid information of the variable is returned.
	 */
	virtual bool isValid() const = 0;
	/**
	 * @brief	With this call, the valid information of the variable is set.
	 */
	// why not just use a boolean here?
	virtual void setValid(uint8_t validity) = 0;

};

using pool_rwm_t = PoolVariableIF::ReadWriteMode_t;

#endif /* POOLVARIABLEIF_H_ */
