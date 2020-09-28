#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_


#include "HasLocalDataPoolIF.h"
#include "../datapool/DataSetIF.h"
#include "../datapool/PoolDataSetBase.h"
#include "../serialize/SerializeIF.h"

#include <vector>

class LocalDataPoolManager;
class PeriodicHousekeepingHelper;

/**
 * @brief	The LocalDataSet class manages a set of locally checked out
 *          variables for local data pools
 * @details
 * This class manages a list, where a set of local variables (or pool variables)
 * are registered. They are checked-out (i.e. their values are looked
 * up and copied) with the read call. After the user finishes working with the
 * pool variables, he can write back all variable values to the pool with
 * the commit call. The data set manages locking and freeing the local data pools,
 * to ensure thread-safety.
 *
 * An internal state manages usage of this class. Variables may only be
 * registered before the read call is made, and the commit call only
 * after the read call.
 *
 * If pool variables are writable and not committed until destruction
 * of the set, the DataSet class automatically sets the valid flag in the
 * data pool to invalid (without) changing the variable's value.
 *
 * @ingroup data_pool
 */
class LocalPoolDataSetBase: public PoolDataSetBase {
	friend class LocalDataPoolManager;
	friend class PeriodicHousekeepingHelper;
public:
	/**
	 * @brief	Constructor for the creator of local pool data.
	 * @details
	 * This constructor also initializes the components required for
	 * periodic handling.
	 */
	LocalPoolDataSetBase(HasLocalDataPoolIF *hkOwner,
			uint32_t setId, PoolVariableIF** registeredVariablesArray,
	        const size_t maxNumberOfVariables, bool noPeriodicHandling = false);

	/**
	 * @brief	Constructor for users of local pool data.
	 * @details
	 * @param sid Unique identifier of dataset consisting of object ID and
	 * set ID.
	 * @param registeredVariablesArray
	 * @param maxNumberOfVariables
	 */
	LocalPoolDataSetBase(sid_t sid, PoolVariableIF** registeredVariablesArray,
	        const size_t maxNumberOfVariables);

	/**
	 * @brief	The destructor automatically manages writing the valid
	 * 			information of variables.
	 * @details
	 * In case the data set was read out, but not committed(indicated by state),
	 * the destructor parses all variables that are still registered to the set.
	 * For each, the valid flag in the data pool is set to "invalid".
	 */
	~LocalPoolDataSetBase();

	void setValidityBufferGeneration(bool withValidityBuffer);

	sid_t getSid() const;

	ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
	            SerializeIF::Endianness streamEndianness) const override;
	ReturnValue_t deSerialize(const uint8_t** buffer, size_t *size,
	        SerializeIF::Endianness streamEndianness) override;
	size_t getSerializedSize() const override;

	/**
	 * Special version of the serilization function which appends a
	 * validity buffer at the end. Each bit of this validity buffer
	 * denotes whether the container data set entries are valid from left
	 * to right, MSB first.
	 * @param buffer
	 * @param size
	 * @param maxSize
	 * @param bigEndian
	 * @param withValidityBuffer
	 * @return
	 */
	ReturnValue_t serializeWithValidityBuffer(uint8_t** buffer,
	        size_t* size, size_t maxSize,
	        SerializeIF::Endianness streamEndianness) const;
	ReturnValue_t deSerializeWithValidityBuffer(const uint8_t** buffer,
	        size_t *size, SerializeIF::Endianness streamEndianness);
	ReturnValue_t serializeLocalPoolIds(uint8_t** buffer,
	        size_t* size, size_t maxSize,
	        SerializeIF::Endianness streamEndianness,
	        bool serializeFillCount = true) const;
	uint8_t getLocalPoolIdsSerializedSize(bool serializeFillCount = true) const;

	/**
	 * Set the dataset valid or invalid
	 * @param setEntriesRecursively
	 * If this is true, all contained datasets will also be set recursively.
	 */
	void setValidity(bool valid, bool setEntriesRecursively);
	bool isValid() const override;

	void setChanged(bool changed);
	bool isChanged() const;

protected:
	sid_t sid;

	bool diagnostic = false;
	void setDiagnostic(bool diagnostics);
	bool isDiagnostics() const;

	/**
	 * Used for periodic generation.
	 */
	bool reportingEnabled = false;
	void setReportingEnabled(bool enabled);
	bool getReportingEnabled() const;

	void initializePeriodicHelper(float collectionInterval,
			dur_millis_t minimumPeriodicInterval,
			bool isDiagnostics, uint8_t nonDiagIntervalFactor = 5);

	/**
	 * If the valid state of a dataset is always relevant to the whole
	 * data set we can use this flag.
	 */
	bool valid = false;

	/**
	 * Can be used to mark the dataset as changed, which is used
	 * by the LocalDataPoolManager to send out update messages.
	 */
	bool changed = false;

	bool withValidityBuffer = true;

	/**
	 * @brief	This is a small helper function to facilitate locking
	 * 			the global data pool.
	 * @details
	 * It makes use of the lockDataPool method offered by the DataPool class.
	 */
	ReturnValue_t lockDataPool(uint32_t timeoutMs) override;
	/**
	 * @brief	This is a small helper function to facilitate
	 * 			unlocking the global data pool
	 * @details
	 * It makes use of the freeDataPoolLock method offered by the DataPool class.
	 */
	ReturnValue_t unlockDataPool() override;

	LocalDataPoolManager* hkManager;

	/**
	 * Set n-th bit of a byte, with n being the position from 0
	 * (most significant bit) to 7 (least significant bit)
	 */
	void bitSetter(uint8_t* byte, uint8_t position) const;
	bool bitGetter(const uint8_t* byte, uint8_t position) const;

	PeriodicHousekeepingHelper* periodicHelper = nullptr;

};


#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_ */
