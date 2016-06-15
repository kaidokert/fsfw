/*
 * TmStoreFrontendIF.h
 *
 *  Created on: 19.02.2015
 *      Author: baetz
 */

#ifndef PLATFORM_TMTCSERVICES_TMSTOREFRONTENDIF_H_
#define PLATFORM_TMTCSERVICES_TMSTOREFRONTENDIF_H_

#include <framework/ipc/MessageQueueSender.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
class TmPacketMinimal;
class SpacePacketBase;
class TmStoreBackendIF;

class TmStoreFrontendIF {
public:
	struct ApidSsc {
		uint16_t apid;
		uint16_t ssc;
	};
	virtual TmStoreBackendIF* getBackend() const = 0;
	virtual ReturnValue_t performOperation() = 0;
	/**
	 * Callback from the back-end to indicate a certain packet was received.
	 * front-end takes care of discarding/downloading the packet.
	 * @param packet	Pointer to the newly received Space Packet.
	 * @param address	Start address of the packet found
	 * @param isLastPacket	Indicates if no more packets can be fetched.
	 * @return	If more packets shall be fetched, RETURN_OK must be returned.
	 * 			Any other code stops fetching packets.
	 */
	virtual ReturnValue_t packetRetrieved(TmPacketMinimal* packet, uint32_t address, bool isLastPacket = false) = 0;
	virtual void handleRetrievalFailed(ReturnValue_t errorCode, uint32_t parameter1 = 0, uint32_t parameter2 = 0) = 0;
	/**
	 * To get the queue where commands shall be sent.
	 * @return	Id of command queue.
	 */
	virtual MessageQueueId_t getCommandQueue() = 0;
	virtual ReturnValue_t fetchPackets(ApidSsc start, ApidSsc end) = 0;
	virtual ReturnValue_t deletePackets(ApidSsc upTo) = 0;
	virtual ReturnValue_t checkPacket(SpacePacketBase* tmPacket) = 0;
	virtual bool isEnabled() const = 0;
	virtual void setEnabled(bool enabled) = 0;
	virtual void restDownlinkedPacketCount() = 0;
	static const uint8_t INTERFACE_ID = TM_STORE_FRONTEND_IF;
	static const ReturnValue_t BUSY = MAKE_RETURN_CODE(1);
	static const ReturnValue_t LAST_PACKET_FOUND = MAKE_RETURN_CODE(2);
	static const ReturnValue_t STOP_FETCH = MAKE_RETURN_CODE(3);
	static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(4);
	virtual ~TmStoreFrontendIF() {
	}
};



#endif /* PLATFORM_TMTCSERVICES_TMSTOREFRONTENDIF_H_ */
