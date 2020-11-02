#ifndef TMTCPACKET_PUS_TMPACKETSTORED_H_
#define TMTCPACKET_PUS_TMPACKETSTORED_H_

#include "TmPacketBase.h"

#include "../../serialize/SerializeIF.h"
#include "../../storagemanager/StorageManagerIF.h"
#include "../../internalError/InternalErrorReporterIF.h"
#include "../../ipc/MessageQueueSenderIF.h"

/**
 *	This class generates a ECSS PUS Telemetry packet within a given
 *	intermediate storage.
 *	As most packets are passed between tasks with the help of a storage
 *	anyway, it seems logical to create a Packet-In-Storage access class
 *	which saves the user almost all storage handling operation.
 *	Packets can both be newly created with the class and be "linked" to
 *	packets in a store with the help of a storeAddress.
 *	@ingroup tmtcpackets
 */
class TmPacketStored : public TmPacketBase {
public:
	/**
	 * This is a default constructor which does not set the data pointer.
	 * However, it does try to set the packet store.
	 */
	TmPacketStored( store_address_t setAddress );
	/**
	 * With this constructor, new space is allocated in the packet store and
	 * a new PUS Telemetry Packet is created there.
	 * Packet Application Data passed in data is copied into the packet.
	 * The Application data is passed in two parts, first a header, then a
	 * data field. This allows building a Telemetry Packet from two separate
	 * data sources.
	 * @param apid			Sets the packet's APID field.
	 * @param service		Sets the packet's Service ID field.
	 * 		This specifies the source service.
	 * @param subservice	Sets the packet's Service Subtype field.
	 * 		This specifies the source sub-service.
	 * @param packet_counter	Sets the Packet counter field of this packet
	 * @param data			The payload data to be copied to the
	 * 						Application Data Field
	 * @param size			The amount of data to be copied.
	 * @param headerData 	The header Data of the Application field,
	 * 						 will be copied in front of data
	 * @param headerSize	The size of the headerDataF
	 */
	TmPacketStored( uint16_t apid, uint8_t service,	uint8_t subservice,
			uint8_t packet_counter = 0, const uint8_t* data = nullptr,
			uint32_t size = 0, const uint8_t* headerData = nullptr,
			uint32_t headerSize = 0);
	/**
	 * Another ctor to directly pass structured content and header data to the
	 * packet to avoid additional buffers.
	 */
	TmPacketStored( uint16_t apid, uint8_t service,	uint8_t subservice,
			uint8_t packet_counter, SerializeIF* content,
			SerializeIF* header = nullptr);
	/**
	 * This is a getter for the current store address of the packet.
	 * @return	The current store address. The (raw) value is
	 * 			@c StorageManagerIF::INVALID_ADDRESS if
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

	ReturnValue_t sendPacket( MessageQueueId_t destination,
			MessageQueueId_t sentFrom, bool doErrorReporting = true );
private:
	/**
	 * This is a pointer to the store all instances of the class use.
	 * If the store is not yet set (i.e. @c store is NULL), every constructor
	 * call tries to set it and throws an error message in case of failures.
	 * The default store is objects::TM_STORE.
	 */
	static StorageManagerIF* store;

	static InternalErrorReporterIF *internalErrorReporter;

	/**
	 * The address where the packet data of the object instance is stored.
	 */
	store_address_t storeAddress;
	/**
	 * A helper method to check if a store is assigned to the class.
	 * If not, the method tries to retrieve the store from the global
	 * ObjectManager.
	 * @return	@li	@c true if the store is linked or could be created.
	 * 			@li @c false otherwise.
	 */
	bool checkAndSetStore();

	void checkAndReportLostTm();
};


#endif /* TMTCPACKET_PUS_TMPACKETSTORED_H_ */
