#ifndef POOLRAWACCESS_H_
#define POOLRAWACCESS_H_

#include <framework/datapool/DataSetIF.h>
#include <framework/datapool/PoolVariableIF.h>
#include <framework/globalfunctions/Type.h>

/**
 * This class allows accessing Data Pool variables as raw bytes.
 * This is necessary to have an access method for HK data, as the PID's alone do not
 * provide a type information.
 *	\ingroup data_pool
 */
class PoolRawAccess: public PoolVariableIF {
private:
	/**
	 * \brief	To access the correct data pool entry on read and commit calls, the data pool id
	 * 			is stored.
	 */
	uint32_t dataPoolId;
	/**
	 * \brief	The array entry that is fetched from the data pool.
	 */
	uint8_t arrayEntry;
	/**
	 * \brief	The valid information as it was stored in the data pool is copied to this attribute.
	 */
	uint8_t valid;
	/**
	 *  \brief	This value contains the type of the data pool entry.
	 */
	Type type;
	/**
	 * \brief	This value contains the size of the data pool entry type in bytes.
	 */
	uint8_t typeSize;
	/**
	 * The size of the DP array (single values return 1)
	 */
	uint8_t arraySize;
	/**
	 * The size (in bytes) from the selected entry till the end of this DataPool variable.
	 */
	uint16_t sizeTillEnd;
	/**
	 * \brief	The information whether the class is read-write or read-only is stored here.
	 */
	ReadWriteMode_t readWriteMode;
	static const uint8_t RAW_MAX_SIZE = sizeof(double);
protected:
	/**
	 * \brief	The commit call writes back the variable's value to the data pool.
	 * \details	It checks type and size, as well as if the variable is writable. If so,
	 * 			the value is copied and the valid flag is automatically set to "valid".
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 *
	 */
	ReturnValue_t commit();
public:

	static const uint8_t INTERFACE_ID = CLASS_ID::POOL_RAW_ACCESS_CLASS;
	static const ReturnValue_t INCORRECT_SIZE = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t DATA_POOL_ACCESS_FAILED = MAKE_RETURN_CODE(0x02);
	uint8_t value[RAW_MAX_SIZE];
	PoolRawAccess(uint32_t data_pool_id, uint8_t arrayEntry,
			DataSetIF* data_set, ReadWriteMode_t setReadWriteMode =
					PoolVariableIF::VAR_READ);
	/**
	 * \brief	The classes destructor is empty. If commit() was not called, the local value is
	 * 			discarded and not written back to the data pool.
	 */
	~PoolRawAccess();
	/**
	 * \brief	This is a call to read the value from the global data pool.
	 * \details	When executed, this operation tries to fetch the pool entry with matching
	 * 			data pool id from the global data pool and copies the value and the valid
	 * 			information to its local attributes. In case of a failure (wrong type or
	 * 			pool id not found), the variable is set to zero and invalid.
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 */
	ReturnValue_t read();

	/**
	 * @brief 	Serialize raw pool entry into provided buffer directly
	 * @param buffer Provided buffer. Raw pool data will be copied here
	 * @param size [out] Increment provided size value by serialized size
	 * @param max_size Maximum allowed serialization size
	 * @param bigEndian Specify endianess
	 * @return - @c RETURN_OK if serialization was successfull
	 *         - @c SerializeIF::BUFFER_TOO_SHORT if range check failed
	 */
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	/**
	 * \brief	This operation returns a pointer to the entry fetched.
	 * \details	Return pointer to the buffer containing the raw data
	 *          Size and number of data can be retrieved by other means.
	 */
	uint8_t* getEntry();
	/**
	 * \brief 	This operation returns the fetched entry from the data pool and
	 * 			flips the bytes, if necessary.
	 * \details	It makes use of the getEntry call of this function, but additionally flips the
	 * 			bytes to big endian, which is the default for external communication (as House-
	 * 			keeping telemetry). To achieve this, the data is copied directly to the passed
	 * 			buffer, if it fits in the given max_size.
	 * \param buffer	A pointer to a buffer to write to
	 * \param writtenBytes	The number of bytes written is returned with this value.
	 * \param max_size	The maximum size that the function may write to buffer.
	 * \return	- \c RETURN_OK if entry could be acquired
	 * 			- \c RETURN_FAILED else.
	 */
	ReturnValue_t getEntryEndianSafe(uint8_t* buffer, uint32_t* size,
			uint32_t max_size);
	/**
	 * With this method, the content can be set from a big endian buffer safely.
	 * @param buffer	Pointer to the data to set
	 * @param size		Size of the data to write. Must fit this->size.
	 * @return	- \c RETURN_OK on success
	 * 			- \c RETURN_FAILED on failure
	 */
	ReturnValue_t setEntryFromBigEndian(const uint8_t* buffer,
			uint32_t setSize);
	/**
	 *  \brief This operation returns the type of the entry currently stored.
	 */
	Type getType();
	/**
	 *  \brief This operation returns the size of the entry currently stored.
	 */
	uint8_t getSizeOfType();
	/**
	 *
	 * @return the size of the datapool array
	 */
	uint8_t getArraySize();
	/**
	 * \brief	This operation returns the data pool id of the variable.
	 */
	uint32_t getDataPoolId() const;
	/**
	 * This method returns if the variable is read-write or read-only.
	 */
	ReadWriteMode_t getReadWriteMode() const;
	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const;

	void setValid(uint8_t valid);
	/**
	 * Getter for the remaining size.
	 */
	uint16_t getSizeTillEnd() const;

	uint32_t getSerializedSize() const;

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);
};

#endif /* POOLRAWACCESS_H_ */
