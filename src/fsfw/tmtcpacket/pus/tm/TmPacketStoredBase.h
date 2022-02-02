#ifndef FSFW_TMTCPACKET_PUS_TMPACKETSTOREDBASE_H_
#define FSFW_TMTCPACKET_PUS_TMPACKETSTOREDBASE_H_

#include "fsfw/FSFW.h"
#include "TmPacketBase.h"
#include "TmPacketStoredBase.h"
#include "TmPacketPusA.h"

#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"

/**
 *  This class generates a ECSS PUS Telemetry packet within a given
 *  intermediate storage.
 *  As most packets are passed between tasks with the help of a storage
 *  anyway, it seems logical to create a Packet-In-Storage access class
 *  which saves the user almost all storage handling operation.
 *  Packets can both be newly created with the class and be "linked" to
 *  packets in a store with the help of a storeAddress.
 *  @ingroup tmtcpackets
 */
class TmPacketStoredBase: virtual public RedirectableDataPointerIF {
public:
    /**
     * This is a default constructor which does not set the data pointer.
     * However, it does try to set the packet store.
     */
    TmPacketStoredBase( store_address_t setAddress );
    TmPacketStoredBase();

    virtual ~TmPacketStoredBase();

    virtual uint8_t* getAllTmData() = 0;

    /**
     * This is a getter for the current store address of the packet.
     * @return  The current store address. The (raw) value is
     *          @c StorageManagerIF::INVALID_ADDRESS if
     *          the packet is not linked.
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
     * @param setAddress    The new packet id to link to.
     */
    void setStoreAddress(store_address_t setAddress);

    ReturnValue_t sendPacket(MessageQueueId_t destination, MessageQueueId_t sentFrom,
            bool doErrorReporting = true);

protected:
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
     * @return  @li @c true if the store is linked or could be created.
     *          @li @c false otherwise.
     */
    bool checkAndSetStore();

    void checkAndReportLostTm();

    void handleStoreFailure(const char* const packetType, ReturnValue_t result,
            size_t sizeToReserve);
};


#endif /* FSFW_TMTCPACKET_PUS_TMPACKETSTOREDBASE_H_ */

