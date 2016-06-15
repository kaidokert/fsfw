/*
 * PUSDistributor.h
 *
 *  Created on: 18.06.2012
 *      Author: baetz
 */

#ifndef PUSDISTRIBUTOR_H_
#define PUSDISTRIBUTOR_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tcdistribution/PUSDistributorIF.h>
#include <framework/tcdistribution/TcDistributor.h>
#include <framework/tcdistribution/TcPacketCheck.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
#include <framework/tmtcservices/VerificationReporter.h>

/**
 * This class accepts PUS Telecommands and forwards them to Application services.
 * In addition, the class performs a formal packet check and sends acceptance success
 * or failure messages.
 * \ingroup tc_distribution
 */
class PUSDistributor : public TcDistributor, public PUSDistributorIF, public AcceptsTelecommandsIF {
protected:
	/**
	 * This attribute contains the class, that performs a formal packet check.
	 */
	TcPacketCheck checker;
	/**
	 * With this class, verification messages are sent to the TC Verification service.
	 */
	VerificationReporter verify_channel;
	/**
	 * The currently handled packet is stored here.
	 */
	TcPacketStored current_packet;
	/**
	 * With this variable, the current check status is stored to generate acceptance messages later.
	 */
	ReturnValue_t tc_status;
	/**
	 * This method reads the packet service, checks if such a service is registered and forwards the packet to the destination.
	 * It also initiates the formal packet check and sending of verification messages.
	 * @return Iterator to map entry of found service id or iterator to \c map.end().
	 */
	iterator_t selectDestination();
	/**
	 * The callback here handles the generation of acceptance success/failure messages.
	 */
	ReturnValue_t callbackAfterSending( ReturnValue_t queueStatus );
public:
	/**
	 * The ctor passes \c set_apid to the checker class and calls the TcDistribution ctor with a certain object id.
	 * @param set_apid The APID of this receiving Application.
	 */
	PUSDistributor( uint16_t set_apid );
	/**
	 * The destructor is empty.
	 */
	virtual ~PUSDistributor();
	ReturnValue_t registerService( AcceptsTelecommandsIF* service );
	MessageQueueId_t getRequestQueue();
	uint16_t getIdentifier();
	ReturnValue_t initialize();
};




#endif /* PUSDISTRIBUTOR_H_ */
