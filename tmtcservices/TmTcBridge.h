#ifndef FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_
#define FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_


#include "../objectmanager/SystemObject.h"
#include "../tmtcservices/AcceptsTelemetryIF.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../ipc/MessageQueueIF.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"
#include "../container/DynamicFIFO.h"
#include "../tmtcservices/TmTcMessage.h"

class TmTcBridge : public AcceptsTelemetryIF,
		public AcceptsTelecommandsIF,
		public ExecutableObjectIF,
		public HasReturnvaluesIF,
		public SystemObject {
public:
	static constexpr uint8_t TMTC_RECEPTION_QUEUE_DEPTH = 20;
	static constexpr uint8_t LIMIT_STORED_DATA_SENT_PER_CYCLE = 15;
	static constexpr uint8_t LIMIT_DOWNLINK_PACKETS_STORED = 20;

	static constexpr uint8_t DEFAULT_STORED_DATA_SENT_PER_CYCLE = 5;
	static constexpr uint8_t DEFAULT_DOWNLINK_PACKETS_STORED = 10;

	TmTcBridge(object_id_t objectId, object_id_t tcDestination,
			object_id_t tmStoreId, object_id_t tcStoreId);
	virtual ~TmTcBridge();

	/**
	 * Set number of packets sent per performOperation().Please note that this
	 * value must be smaller than MAX_STORED_DATA_SENT_PER_CYCLE
	 * @param sentPacketsPerCycle
	 * @return -@c RETURN_OK if value was set successfully
	 * 		   -@c RETURN_FAILED otherwise, stored value stays the same
	 */
	ReturnValue_t setNumberOfSentPacketsPerCycle(uint8_t sentPacketsPerCycle);

	/**
	 * Set number of packets sent per performOperation().Please note that this
	 * value must be smaller than MAX_DOWNLINK_PACKETS_STORED
	 * @param sentPacketsPerCycle
	 * @return -@c RETURN_OK if value was set successfully
	 *         -@c RETURN_FAILED otherwise, stored value stays the same
	 */
	ReturnValue_t setMaxNumberOfPacketsStored(uint8_t maxNumberOfPacketsStored);

	/**
	 * This will set up the bridge to overwrite old data in the FIFO.
	 * @param overwriteOld
	 */
	void setFifoToOverwriteOldData(bool overwriteOld);

	virtual void registerCommConnect();
	virtual void registerCommDisconnect();

	/**
	 * Initializes necessary FSFW components for the TMTC Bridge
	 * @return
	 */
	virtual ReturnValue_t initialize() override;

	/**
	 * @brief	Handles TMTC reception
	 */
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0) override;


	/** AcceptsTelemetryIF override */
	virtual MessageQueueId_t getReportReceptionQueue(
			uint8_t virtualChannel = 0) override;

	/** AcceptsTelecommandsIF override */
	virtual uint16_t getIdentifier() override;
	virtual MessageQueueId_t getRequestQueue() override;

protected:
	//! Cached for initialize function.
	object_id_t tmStoreId = objects::NO_OBJECT;
	object_id_t tcStoreId = objects::NO_OBJECT;
	object_id_t tcDestination = objects::NO_OBJECT;

	//! Used to send and receive TMTC messages.
	//! The TmTcMessage class is used to transport messages between tasks.
	MessageQueueIF* tmTcReceptionQueue = nullptr;

	StorageManagerIF* tmStore = nullptr;
	StorageManagerIF* tcStore = nullptr;

	//! Used to specify whether communication link is up. Will be true
	//! by default, so telemetry will be handled immediately.
	bool communicationLinkUp = true;
	bool tmStored = false;
	bool overwriteOld = true;
	uint8_t packetSentCounter = 0;

	/**
	 * @brief 	Handle TC reception
	 * @details
	 * Default implementation provided, but is empty.
	 * In most cases, TC reception will be handled in a separate task anyway.
	 * @return
	 */
	virtual ReturnValue_t handleTc();

	/**
	 * Handle Telemetry. Default implementation provided.
	 * Calls sendTm()
	 * @return
	 */
	virtual ReturnValue_t handleTm();

	/**
	 * Read the TM Queue and send TM if necessary.
	 * Default implementation provided
	 * @return
	 */
	virtual ReturnValue_t handleTmQueue();

	/**
	 * Send stored data if communication link is active
	 * @return
	 */
	virtual ReturnValue_t handleStoredTm();

	/**
	 * Implemented by child class. Perform sending of Telemetry by implementing
	 * communication drivers or wrappers, e.g. serial communication or a socket
	 * call.
	 * @param data
	 * @param dataLen
	 * @return
	 */
	virtual ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) = 0;

	/**
	 * Store data to be sent later if communication link is not up.
	 * @param message
	 * @return
	 */
	virtual ReturnValue_t storeDownlinkData(TmTcMessage * message);


	/**
	 * Print data as hexidecimal array
	 * @param data
	 * @param dataLen
	 */
	void printData(uint8_t * data, size_t dataLen);

	/**
	 * This fifo can be used to store downlink data
	 * which can not be sent at the moment.
	 */
	DynamicFIFO<store_address_t>* tmFifo = nullptr;
    uint8_t sentPacketsPerCycle = DEFAULT_STORED_DATA_SENT_PER_CYCLE;
    uint8_t maxNumberOfPacketsStored = DEFAULT_DOWNLINK_PACKETS_STORED;
};


#endif /* FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_ */
