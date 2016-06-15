/*
 * CCSDSDistributor.h
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#ifndef CCSDSDISTRIBUTOR_H_
#define CCSDSDISTRIBUTOR_H_

#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tcdistribution/CCSDSDistributorIF.h>
#include <framework/tcdistribution/TcDistributor.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
/**
 * An instantiation of the CCSDSDistributorIF.
 * It receives Space Packets, and selects a destination depending on the APID of the telecommands.
 * The Secondary Header (with Service/Subservice) is ignored.
 * \ingroup tc_distribution
 */
class CCSDSDistributor : public TcDistributor, public CCSDSDistributorIF, public AcceptsTelecommandsIF {
protected:
	/**
	 * This implementation checks if an Application with fitting APID has registered and forwards the
	 * packet to the according message queue.
	 * If the packet is not found, it returns the queue to \c default_apid, where a Acceptance Failure
	 * message should be generated.
	 * @return Iterator to map entry of found APID or iterator to default APID.
	 */
	iterator_t selectDestination();
	/**
	 * The default APID, where packets with unknown APID are sent to.
	 */
	uint16_t default_apid;
	/**
	 * A reference to the TC storage must be maintained, as this class handles pure Space Packets and there
	 * exists no SpacePacketStored class.
	 */
	StorageManagerIF* tcStore;
	/**
	 * The callback here handles the generation of acceptance success/failure messages.
	 */
	ReturnValue_t callbackAfterSending( ReturnValue_t queueStatus );
public:
	/**
	 * The constructor sets the default APID and calls the TcDistributor ctor with a certain object id.
	 * \c tcStore is set in the \c initialize method.
	 * @param set_default_apid The default APID, where packets with unknown destination are sent to.
	 */
	CCSDSDistributor( uint16_t set_default_apid = 0x35 );
	/**
	 * The destructor is empty.
	 */
	~CCSDSDistributor();
	MessageQueueId_t getRequestQueue();
	ReturnValue_t registerApplication( uint16_t apid, MessageQueueId_t id );
	ReturnValue_t registerApplication( AcceptsTelecommandsIF* application );
	uint16_t getIdentifier();
	ReturnValue_t initialize();
};



#endif /* CCSDSDISTRIBUTOR_H_ */
