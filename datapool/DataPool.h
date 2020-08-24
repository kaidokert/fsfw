/**
 *	\file	DataPool.h
 *
 *  \date	10/17/2012
 *	\author	Bastian Baetz
 *
 *	\brief	This file contains the definition of the DataPool class and (temporarily)
 *			the "extern" definition of the global dataPool instance.
 */

#ifndef DATAPOOL_H_
#define DATAPOOL_H_

#include "PoolEntry.h"
#include "../globalfunctions/Type.h"
#include "../ipc/MutexIF.h"
#include <map>

/**
 * \defgroup data_pool Data Pool
 * This is the group, where all classes associated with Data Pool Handling belong to.
 * This includes classes to access Data Pool variables.
 */

#define DP_SUCCESSFUL	0
#define DP_FAILURE		1

/**
 *	\brief		This class represents the OBSW global data-pool.
 *
 *	\details	All variables are registered and space is allocated in an initialization
 *				function, which is passed do the constructor.
 *				Space for the variables is allocated on the heap (with a new call).
 *				The data is found by a data pool id, which uniquely represents a variable.
 *				Data pool variables should be used with a blackboard logic in mind,
 *				which means read data is valid (if flagged so), but not necessarily up-to-date.
 *				Variables are either single values or arrays.
 *	\ingroup data_pool
 */
class DataPool : public HasReturnvaluesIF {
private:
	/**
	 * \brief	This is the actual data pool itself.
	 * \details	It is represented by a map
	 * 			with the data pool id as index and a pointer to a single PoolEntry as value.
	 */
	std::map<uint32_t, PoolEntryIF*> data_pool;
public:
	/**
	 * \brief	The mutex is created in the constructor and makes access mutual exclusive.
	 * \details	Locking and unlocking the pool is only done by the DataSet class.
	 */
	MutexIF*	mutex;
	/**
	 * \brief	In the classes constructor, the passed initialization function is called.
	 * \details	To enable filling the pool,
	 * 			a pointer to the map is passed, allowing direct access to the pool's content.
	 * 			On runtime, adding or removing variables is forbidden.
	 */
	DataPool( void ( *initFunction )( std::map<uint32_t, PoolEntryIF*>* pool_map ) );
	/**
	 * 	\brief	The destructor iterates through the data_pool map and calls all Entries destructors to clean up the heap.
	 */
	~DataPool();
	/**
	 * \brief	This is the default call to access the pool.
	 * \details	A pointer to the PoolEntry object is returned.
	 *  		The call checks data pool id, type and array size. Returns NULL in case of failure.
	 * \param data_pool_id	The data pool id to search.
	 * \param sizeOrPosition The array size (not byte size!) of the pool entry, or the position the user wants to read.
	 * 			If smaller than the entry size, everything's ok.
	 */
	template <typename T> PoolEntry<T>* getData( uint32_t data_pool_id, uint8_t sizeOrPosition );
	/**
	 * \brief	An alternative call to get a data pool entry in case the type is not implicitly known
	 * 			(i.e. in Housekeeping Telemetry).
	 * \details	It returns a basic interface and does NOT perform
	 * 			a size check. The caller has to assure he does not copy too much data.
	 * 			Returns NULL in case the entry is not found.
	 * \param data_pool_id	The data pool id to search.
	 */
	PoolEntryIF* getRawData( uint32_t data_pool_id );
	/**
	 * \brief	This is a small helper function to facilitate locking the global data pool.
	 * \details	It fetches the pool's mutex id and tries to acquire the mutex.
	 */
	ReturnValue_t lockDataPool();
	/**
	 * \brief	This is a small helper function to facilitate unlocking the global data pool.
	 * \details	It fetches the pool's mutex id and tries to free the mutex.
	 */
	ReturnValue_t freeDataPoolLock();
	/**
	 * \brief	The print call is a simple debug method.
	 * \details	It prints the current content of the data pool.
	 *  		It iterates through the data_pool map and calls each entry's print() method.
	 */
	void print();
	/**
	 * Extracts the data pool id from a SCOS 2000 PID.
	 * @param parameter_id The passed Parameter ID.
	 * @return The data pool id as used within the OBSW.
	 */
	static uint32_t PIDToDataPoolId( uint32_t parameter_id );
	/**
	 * Extracts an array index out of a SCOS 2000 PID.
	 * @param parameter_id The passed Parameter ID.
	 * @return The index of the corresponding data pool entry.
	 */
	static uint8_t PIDToArrayIndex( uint32_t parameter_id );
	/**
	 * Retransforms a data pool id and an array index to a SCOS 2000 PID.
	 */
	static uint32_t poolIdAndPositionToPid( uint32_t poolId, uint8_t index );

	/**
	 * Method to return the type of a pool variable.
	 * @param parameter_id A parameterID (not pool id) of a DP member.
	 * @param type Returns the type or TYPE::UNKNOWN_TYPE
	 * @return RETURN_OK if parameter exists, RETURN_FAILED else.
	 */
	ReturnValue_t getType( uint32_t parameter_id, Type* type );

	/**
	 * Method to check if a PID exists.
	 * Does not lock, as there's no possibility to alter the list that is checked during run-time.
	 * @param parameterId The PID (not pool id!) of a parameter.
	 * @return	true if exists, false else.
	 */
	bool exists(uint32_t parameterId);
};

//We assume someone globally instantiates a DataPool.
extern DataPool dataPool;
#endif /* DATAPOOL_H_ */
