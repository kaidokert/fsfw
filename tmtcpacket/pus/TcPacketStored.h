#ifndef TCPACKETSTORED_H_
#define TCPACKETSTORED_H_

#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tmtcpacket/pus/TcPacketBase.h>

/**
 *	This class generates a ECSS PUS Telecommand packet within a given
 *	intermediate storage.
 *	As most packets are passed between tasks with the help of a storage
 *	anyway, it seems logical to create a Packet-In-Storage access class
 *	which saves the user almost all storage handling operation.
 *	Packets can both be newly created with the class and be "linked" to
 *	packets in a store with the help of a storeAddress.
 *	@ingroup tmtcpackets
 */
class TcPacketStored : public TcPacketBase {
private:
	/**
	 * This is a pointer to the store all instances of the class use.
	 * If the store is not yet set (i.e. \c store is NULL), every constructor
	 * call tries to set it and throws an error message in case of failures.
	 * The default store is objects::TC_STORE.
	 */
	static StorageManagerIF* store;
	/**
	 * The address where the packet data of the object instance is stored.
	 */
	store_address_t storeAddress;
	/**
	 * A helper method to check if a store is assigned to the class.
	 * If not, the method tries to retrieve the store from the global
	 * ObjectManager.
	 * @return	@li	\c true if the store is linked or could be created.
	 * 			@li \c false otherwise.
	 */
	bool checkAndSetStore();
public:
	/**
	 * This is a default constructor which does not set the data pointer.
	 * However, it does try to set the packet store.
	 */
	TcPacketStored();
	/**
	 * With this constructor, the class instance is linked to an existing
	 * packet in the packet store.
	 * The packet content is neither checked nor changed with this call. If
	 * the packet could not be found, the data pointer is set to NULL.
	 */
	TcPacketStored( store_address_t setAddress );
	/**
	 * With this constructor, new space is allocated in the packet store and
	 * a new PUS Telecommand Packet is created there.
	 * Packet Application Data passed in data is copied into the packet.
	 * @param apid			Sets the packet's APID field.
	 * @param ack			Set's the packet's Ack field,
	 * 		which specifies number and size of verification packets returned
	 * 		for this command.
	 * @param service		Sets the packet's Service ID field.
	 * 		This specifies the destination service.
	 * @param subservice	Sets the packet's Service Subtype field.
	 * 		This specifies the destination sub-service.
	 * @param sequence_count	Sets the packet's Source Sequence Count field.
	 * @param data		The data to be copied to the Application Data Field.
	 * @param size		The amount of data to be copied.
	 */
	TcPacketStored( uint16_t apid, uint8_t ack, uint8_t service, uint8_t subservice, uint8_t sequence_count = 0, const uint8_t* data = NULL, uint32_t size = 0 );
	/**
	 * Another constructor to create a TcPacket from a raw packet stream.
	 * Takes the data and adds it unchecked to the TcStore.
	 * @param data	Pointer to the complete TC Space Packet.
	 * @param Size size of the packet.
	 */
	TcPacketStored( const uint8_t* data, uint32_t size);
	/**
	 * This is a getter for the current store address of the packet.
	 * @return	The current store address. The (raw) value is \c StorageManagerIF::INVALID_ADDRESS if
	 * 			the packet is not linked.
	 */
	store_address_t getStoreAddress();
	/**
	 * With this call, the packet is deleted.
	 * It removes itself from the store and sets its data pointer to NULL.
	 */
	void deletePacket();
	/**
	 * With this call, a packet can be linked to another store. This is useful
	 * if the packet is a class member and used for more than one packet.
	 * @param setAddress	The new packet id to link to.
	 */
	void setStoreAddress( store_address_t setAddress );
	/**
	 * This method performs a size check.
	 * It reads the stored size and compares it with the size entered in the
	 * packet header. This class is the optimal place for such a check as it
	 * has access to both the header data and the store.
	 * @return	true if size is correct, false if packet is not registered in
	 * 			store or size is incorrect.
	 */
	bool isSizeCorrect();
};


#endif /* TCPACKETSTORED_H_ */
