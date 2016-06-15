/*
 * TmStore.h
 *
 *  Created on: 05.02.2015
 *      Author: baetz
 */

#ifndef PLATFORM_TMTCSERVICES_TMSTORE_H_
#define PLATFORM_TMTCSERVICES_TMSTORE_H_

#include <framework/container/FixedArrayList.h>
#include <framework/container/RingBufferBase.h>
#include <framework/events/EventReportingProxyIF.h>
#include <framework/globalfunctions/crc_ccitt.h>
#include <framework/ipc/MessageQueue.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/timemanager/Countdown.h>
#include <framework/tmstorage/TmStoreBackendIF.h>
#include <framework/tmstorage/TmStoreFrontendIF.h>
#include <framework/tmstorage/TmStoreInfo.h>
#include <framework/tmstorage/TmStorePackets.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

class TmStore: public HasReturnvaluesIF, public TmStoreBackendIF {
public:
	static const uint32_t DEFAULT_TIMEOUT_MS = 3000;
	static const uint32_t UPDATE_REMOTE_PTRS_DEFAULT_MS = 20000;
	static const uint32_t MAX_DUMP_PACKETS_PER_CYCLE_DEFAULT = 16;
	static const uint32_t MAX_DUMPED_BYTES_PER_CYCLE_DEFAULT = 1024;
	TmStore(TmStoreFrontendIF* owner, object_id_t memoryObject,
			uint32_t storeAddress, uint32_t size, bool overwriteOld = false,
			uint32_t maxDumpPacketsPerCylce = MAX_DUMP_PACKETS_PER_CYCLE_DEFAULT,
			uint32_t maxDumpedBytesPerCycle = MAX_DUMPED_BYTES_PER_CYCLE_DEFAULT,
			uint32_t updateRemotePtrsMs = UPDATE_REMOTE_PTRS_DEFAULT_MS,
			uint32_t chunkSize = CHUNK_SIZE_DEFAULT);
	virtual ~TmStore();
	ReturnValue_t performOperation();
	ReturnValue_t storePacket(TmPacketMinimal* tmPacket);
	ReturnValue_t fetchPackets(bool useAddress = false,
			uint32_t startAtAddress = 0);
	uint32_t availableData();
	ReturnValue_t deleteContent(bool deletePart = false, uint32_t upToAddress =
			0, uint32_t nDeletedPackets = 0, TmPacketMinimal* newOldestPacket =
			NULL);
	ReturnValue_t initialize();
	ReturnValue_t initializeStore();
	bool isReady();
	void resetStore(bool resetWrite = false, bool resetRead = false);
	float getPercentageFilled() const;
	uint32_t getStoredPacketsCount() const;
	TmPacketInformation* getOldestPacket();
	TmPacketInformation* getYoungestPacket();
private:
	TmStoreFrontendIF* owner;
	EventReportingProxyIF* eventProxy;
	TmPacketInformation pendingOldestPacket;
	TmPacketInformation pendingNewestPacket;
	TmPacketInformation oldestPacket;
	TmPacketInformation newestPacket;
	TmStoreInfo info;
	const uint32_t infoSize;
	RingBufferBase<2> ring;
	enum State {
		OFF = 0,
		STARTUP = 1,
		FETCH_STORE_INFORMATION = 2,
		STORE_INFORMATION_RECEIVED = 3,
		READY = 4
	};
	State state;
	enum WriteState {
		WRITE_IDLE = 0, SETTING_STORE_INFO = 1, SENDING_PACKETS = 2,
	};
	WriteState writeState;
	enum ReadState {
		READ_IDLE = 0,
		DELETING_OLD = 1,
		DUMPING_PACKETS = 2,
		FETCHING_STORE_INFO = 3,
		DELETING_MORE = 4
	};
	ReadState readState;
	object_id_t memoryObject;
	MessageQueue memoryQueue;
	StorageManagerIF* ipcStore;
	Countdown timer;
	uint32_t pendingDataToWrite;
	uint32_t maximumAmountToRead;
	uint32_t storedPacketCounter;
	uint32_t pendingStoredPackets;
	bool splitWrite;
	store_address_t splitWriteStoreId;
	enum SplitReadState {
		NO_SPLIT, SPLIT_STARTED, ONE_RECEIVED
	};
	SplitReadState splitRead;
	uint32_t splitReadTotalSize;
	ArrayList<uint8_t, uint16_t> tmBuffer;
	uint8_t* dumpBuffer;
	const uint32_t pointerAddress;
	const uint32_t updateRemotePtrsMs;
	const uint32_t maxDumpPacketsPerCycle;
	const uint32_t maxDumpedBytesPerCycle;
	Countdown localBufferTimer;
	bool tryToSetStoreInfo;
	static const uint8_t READ_PTR = 0;
	static const uint8_t TEMP_READ_PTR = 1;
	static const uint32_t LOCAL_BUFFER_TIMEOUT_MS = 5000;
	static const uint32_t CHUNK_SIZE_DEFAULT = 2048;
	void checkMemoryQueue();
	ReturnValue_t requestStoreInfo();
	ReturnValue_t setStoreInfo();
	void readStoreInfo(const uint8_t* buffer, uint32_t size);
	void dumpPackets(const uint8_t* buffer, uint32_t size);
	/**
	 * Deletes old packets to free space in the store for new packets.
	 * Two chunks of data are requested to definitely delete mored than one
	 * chunk. Deletion is stopped when the last full packet is found, to
	 * be able to remember the oldest packet in store.
	 * @param buffer Data containing the old packets.
	 * @param size	Size of the data chunk.
	 */
	void deleteOld(const uint8_t* buffer, uint32_t size);
	void doStateMachine();
	void handleLoadSuccess();
	void handleDump(store_address_t storeId);
	void handleLoadFailed(ReturnValue_t errorCode);
	void handleDumpFailed(ReturnValue_t errorCode);
	ReturnValue_t sendDataToTmStore(const uint8_t* data, uint32_t size);
	ReturnValue_t sendRemainingTmPiece();
	ReturnValue_t requestChunkOfData();
	ReturnValue_t requestRemainingDumpPiece(uint32_t firstPartSize);
	ReturnValue_t storeOrForwardPacket(const uint8_t* data, uint32_t size);
	ReturnValue_t handleFullStore(const uint8_t* data, uint32_t size);
	bool hasEnoughSpaceFor(uint32_t size);
	void sendTmBufferToStore();
	void handleSplitRead(const uint8_t* buffer, uint32_t size);
	void setState(State state);
	void setReadState(ReadState readState);
	void setWriteState(WriteState writeState);
	uint32_t chunkSize();
	void clearPending();
};

#endif /* PLATFORM_TMTCSERVICES_TMSTORE_H_ */
