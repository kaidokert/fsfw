/*
 * TmStoreManager.h
 *
 *  Created on: 18.02.2015
 *      Author: baetz
 */

#ifndef PLATFORM_TMTCSERVICES_TMSTOREMANAGER_H_
#define PLATFORM_TMTCSERVICES_TMSTOREMANAGER_H_

#include <framework/ipc/MessageQueue.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/timemanager/Countdown.h>
#include <framework/tmstorage/TmStoreBackendIF.h>
#include <framework/tmstorage/TmStoreFrontendIF.h>
#include <framework/tmstorage/TmStorePackets.h>
#include <framework/tmtcpacket/packetmatcher/PacketMatchTree.h>

class TmStoreManager : public SystemObject, public HasReturnvaluesIF, public TmStoreFrontendIF {
public:
	TmStoreManager(object_id_t objectId, object_id_t setDumpTarget, uint8_t setVC, uint32_t setTimeoutMs);
	virtual ~TmStoreManager();
	ReturnValue_t fetchPackets(ApidSsc start, ApidSsc end);
	ReturnValue_t deletePackets(ApidSsc upTo);
	ReturnValue_t checkPacket(SpacePacketBase* tmPacket);
	ReturnValue_t performOperation();
	ReturnValue_t initialize();
	ReturnValue_t packetRetrieved(TmPacketMinimal* packet, uint32_t address, bool isLastPacket = false);
	void handleRetrievalFailed(ReturnValue_t errorCode, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
	bool isEnabled() const;
	void setEnabled(bool enabled);
	void setBackend(TmStoreBackendIF* backend);
	TmStoreBackendIF* getBackend() const;
	MessageQueueId_t getCommandQueue();
	ReturnValue_t updateMatch(bool addSelection, uint16_t apid, uint8_t serviceType, uint8_t serviceSubtype);
private:
	MessageQueue commandQueue;
	TmStoreBackendIF* backend;
	StorageManagerIF* tmForwardStore;
	MessageQueueSender tmQueue;
	const object_id_t dumpTarget;
	const uint8_t virtualChannel;
	enum FetchState {
		NOTHING_FETCHED,
		BEFORE_RANGE,
		IN_RANGE
	};
	FetchState fetchState;
	ApidSsc firstPacketToFetch;
	ApidSsc lastPacketToFetch;
	uint32_t addressOfFetchCandidate;
	ApidSsc fetchCandidate;
	bool deletionStarted;
	ApidSsc lastPacketToDelete;
	uint32_t lastAddressToDelete;
	uint32_t pendingPacketsToDelete;
	TmPacketInformation pendingOldestPacket;
	enum State {
		IDLE,
		DELETING_PACKETS,
		RETRIEVING_PACKETS,
		GET_OLDEST_PACKET_INFO
	};
	State state;
	bool storingEnabled;
	const uint32_t timeoutMs;
	Countdown timer;
	PacketMatchTree matcher;
	StorageManagerIF* ipcStore;
	SerializeElement<uint32_t> downlinkedPacketsCount;
	bool fullEventThrown;
	ReturnValue_t checkRetrievalLimit(TmPacketMinimal* packet, uint32_t address);
	ReturnValue_t checkDeletionLimit(TmPacketMinimal* packet, uint32_t address);
	void dumpPacket(SpacePacketBase* packet);
	void checkCommandQueue();
	void replySuccess();
	void replyFailure(ReturnValue_t errorCode, uint32_t parameter = 0);
	ReturnValue_t changeSelectionDefinition(bool addSelection, store_address_t storeId, uint32_t* errorCount);
	ReturnValue_t sendMatchTree();
	void restDownlinkedPacketCount();
	ReturnValue_t sendStatistics();
	static const uint8_t NO_SERVICE = 0;
	static const uint8_t NO_SUBSERVICE = 0;
};

#endif /* PLATFORM_TMTCSERVICES_TMSTOREMANAGER_H_ */
