#ifndef STORAGEMANAGERIF_H_H
#define STORAGEMANAGERIF_H_H

#include <framework/events/Event.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <stddef.h>

/**
 * This union defines the type that identifies where a data packet is stored in the store.
 * It comprises of a raw part to read it as raw value and a structured part to use it in
 * pool-like stores.
 */
union store_address_t {
	/**
	 * Default Constructor, initializing to INVALID_ADDRESS
	 */
	store_address_t():raw(0xFFFFFFFF){}
	/**
	 * Constructor to create an address object using the raw address
	 *
	 * @param rawAddress
	 */
	store_address_t(uint32_t rawAddress):raw(rawAddress){}

	/**
	 * Constructor to create an address object using pool
	 * and packet indices
	 *
	 * @param poolIndex
	 * @param packetIndex
	 */
	store_address_t(uint16_t poolIndex, uint16_t packetIndex):
		pool_index(poolIndex),packet_index(packetIndex){}
	/**
	 * A structure with two elements to access the store address pool-like.
	 */
	struct {
		/**
		 * The index in which pool the packet lies.
		 */
		uint16_t pool_index;
		/**
		 * The position in the chosen pool.
		 */
		uint16_t packet_index;
	};
	/**
	 * Alternative access to the raw value.
	 */
	uint32_t raw;
};

/**
 * @brief	This class provides an interface for intermediate data storage.
 * @details	The Storage manager classes shall be used to store larger chunks of
 * 			data in RAM for exchange between tasks. This interface expects the
 * 			data to be stored in one consecutive block of memory, so tasks can
 * 			write directly to the destination pointer.
 * 			For interprocess communication, the stores must be locked during
 * 			insertion and deletion. If the receiving storage identifier is
 * 			passed token-like between tasks, a lock during read and write
 * 			operations is not necessary.
 * @author	Bastian Baetz
 * @date	18.09.2012
 */
class StorageManagerIF : public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::STORAGE_MANAGER_IF; //!< The unique ID for return codes for this interface.
	static const ReturnValue_t DATA_TOO_LARGE = MAKE_RETURN_CODE(1); //!< This return code indicates that the data to be stored is too large for the store.
	static const ReturnValue_t DATA_STORAGE_FULL = MAKE_RETURN_CODE(2); //!< This return code indicates that a data storage is full.
	static const ReturnValue_t ILLEGAL_STORAGE_ID = MAKE_RETURN_CODE(3); //!< This return code indicates that data was requested with an illegal storage ID.
	static const ReturnValue_t DATA_DOES_NOT_EXIST = MAKE_RETURN_CODE(4); //!< This return code indicates that the requested ID was valid, but no data is stored there.
	static const ReturnValue_t ILLEGAL_ADDRESS = MAKE_RETURN_CODE(5);

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::OBSW;
	static const Event GET_DATA_FAILED = MAKE_EVENT(0, SEVERITY::LOW);
	static const Event STORE_DATA_FAILED = MAKE_EVENT(1, SEVERITY::LOW);

	static const uint32_t INVALID_ADDRESS = 0xFFFFFFFF; //!< Indicates an invalid (i.e unused) storage address.
	/**
	 * @brief This is the empty virtual destructor as required for C++ interfaces.
	 */
	virtual ~StorageManagerIF() {
	}
	;
	/**
	 * @brief	With addData, a free storage position is allocated and data
	 * 			stored there.
	 * @details	During the allocation, the StorageManager is blocked.
	 * @param storageId A pointer to the storageId to retrieve.
	 * @param data	The data to be stored in the StorageManager.
	 * @param size	The amount of data to be stored.
	 * @return	Returns @li RETURN_OK if data was added.
	 * 					@li	RETURN_FAILED if data could not be added.
	 * 						storageId is unchanged then.
	 */
	virtual ReturnValue_t addData(store_address_t* storageId, const uint8_t * data, uint32_t size, bool ignoreFault = false) = 0;
	/**
	 * @brief	With deleteData, the storageManager frees the memory region
	 * 			identified by packet_id.
	 * @param packet_id	The identifier of the memory region to be freed.
	 * @return	@li RETURN_OK on success.
	 * 			@li	RETURN_FAILED if deletion did not work
	 * 				(e.g. an illegal packet_id was passed).
	 */
	virtual ReturnValue_t deleteData(store_address_t packet_id) = 0;
	/**
	 * @brief	Another deleteData which uses the pointer and size of the stored data to delete the content.
	 * @param buffer	Pointer to the data.
	 * @param size		Size of data to be stored.
	 * @param storeId	Store id of the deleted element (optional)
	 * @return	@li RETURN_OK on success.
	 * 			@li	failure code if deletion did not work
	 */
	virtual ReturnValue_t deleteData(uint8_t* buffer, uint32_t size, store_address_t* storeId = NULL) = 0;
	/**
	 * @brief	getData returns an address to data and the size of the data
	 * 			for a given packet_id.
	 * @param packet_id		The id of the data to be returned
	 * @param packet_ptr	The passed pointer address is set to the the memory
	 * 						position
	 * @param size			The exact size of the stored data is returned here.
	 * @return	@li RETURN_OK on success.
	 * 			@li	RETURN_FAILED if fetching data did not work
	 * 				(e.g. an illegal packet_id was passed).
	 */
	virtual ReturnValue_t getData(store_address_t packet_id,
			const uint8_t** packet_ptr, uint32_t* size) = 0;
	/**
	 * Same as above, but not const and therefore modifiable.
	 */
	virtual ReturnValue_t modifyData(store_address_t packet_id,
			uint8_t** packet_ptr, uint32_t* size) = 0;
	/**
	 * This method reserves an element of \c size.
	 *
	 * It returns the packet id of this element as well as a direct pointer to the
	 * data of the element. It must be assured that exactly \c size data is
	 * written to p_data!
	 * @param storageId A pointer to the storageId to retrieve.
	 * @param size		The size of the space to be reserved.
	 * @param p_data	A pointer to the element data is returned here.
	 * @return	Returns @li RETURN_OK if data was added.
	 * 					@li	RETURN_FAILED if data could not be added.
	 * 						storageId is unchanged then.
	 */
	virtual ReturnValue_t getFreeElement(store_address_t* storageId, const uint32_t size, uint8_t** p_data, bool ignoreFault = false ) = 0;
	/**
	 * Clears the whole store.
	 * Use with care!
	 */
	virtual void clearStore() = 0;

};

#endif /* STORAGEMANAGERIF_H_ */
