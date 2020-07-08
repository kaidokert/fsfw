#ifndef FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_
#define FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_

#include <framework/objectmanager/SystemObject.h>
#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>

#include <framework/container/FIFO.h>
#include <framework/tmtcservices/TmTcMessage.h>

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

	TmTcBridge(object_id_t objectId, object_id_t ccsdsPacketDistributor);
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
	//! Used to send and receive TMTC messages.
	//! TmTcMessage is used to transport messages between tasks.
	MessageQueueIF* tmTcReceptionQueue = nullptr;
	StorageManagerIF* tcStore = nullptr;
	StorageManagerIF* tmStore = nullptr;
	object_id_t ccsdsPacketDistributor = 0;
	//! Used to specify whether communication link is up
	bool communicationLinkUp = false;
	bool tmStored = false;

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
	 * Read the TM Queue and send TM if necessary. Default implementation provided
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
	 * communication drivers or wrappers, e.g. UART communication or lwIP stack.
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
	FIFO<store_address_t, LIMIT_DOWNLINK_PACKETS_STORED> tmFifo;
    uint8_t sentPacketsPerCycle = DEFAULT_STORED_DATA_SENT_PER_CYCLE;
    uint8_t maxNumberOfPacketsStored = DEFAULT_DOWNLINK_PACKETS_STORED;
};


#endif /* FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_ */
