#ifndef FSFW_TMTCPACKET_PUS_TMPACKETSTORED_PUSA_H_
#define FSFW_TMTCPACKET_PUS_TMPACKETSTORED_PUSA_H_

#include "TmPacketBase.h"
#include "TmPacketStoredBase.h"
#include <FSFWConfig.h>

#include "../../tmtcpacket/pus/TmPacketPusA.h"
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
class TmPacketStoredPusA :
        public TmPacketStoredBase,
        public TmPacketPusA {
public:
	/**
	 * This is a default constructor which does not set the data pointer.
	 * However, it does try to set the packet store.
	 */
	TmPacketStoredPusA( store_address_t setAddress );
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
	TmPacketStoredPusA( uint16_t apid, uint8_t service,	uint8_t subservice,
			uint8_t packet_counter = 0, const uint8_t* data = nullptr,
			uint32_t size = 0, const uint8_t* headerData = nullptr,
			uint32_t headerSize = 0);
	/**
	 * Another ctor to directly pass structured content and header data to the
	 * packet to avoid additional buffers.
	 */
	TmPacketStoredPusA( uint16_t apid, uint8_t service,	uint8_t subservice,
			uint8_t packet_counter, SerializeIF* content,
			SerializeIF* header = nullptr);

    uint8_t* getAllTmData() override;
    void setDataPointer(const uint8_t* newPointer) override;

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
};


#endif /* FSFW_TMTCPACKET_PUS_TMPACKETSTORED_PUSA_H_ */
