/**
 * @file TmTcBridge.h
 *
 * @date 26.12.2019
 */

#ifndef FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_
#define FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_

#include <framework/tmtcservices/AcceptsTelemetryIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/objectmanager/SystemObject.h>

#include <framework/tmtcservices/TmTcMessage.h>
#include <framework/container/FIFO.h>

class TmTcBridge : public AcceptsTelemetryIF,
		public ExecutableObjectIF,
		public HasReturnvaluesIF,
		public SystemObject {
public:
	TmTcBridge(object_id_t objectId_, object_id_t ccsdsPacketDistributor_);
	virtual ~TmTcBridge();

	/**
	 * Initializes basic FSFW components for TMTC Bridge
	 * @return
	 */
	virtual ReturnValue_t initialize();

	/**
	 * @brief	The performOperation method is executed in a task.
	 * @details	There are no restrictions for calls within this method, so any
	 * 			other member of the class can be used.
	 * @return	Currently, the return value is ignored.
	 */
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

	/**
	 * Return TMTC Reception Queue
	 * @param virtualChannel
	 * @return
	 */
	virtual MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0);

protected:
	MessageQueueIF* TmTcReceptionQueue; //!< Used to send and receive TMTC messages. TmTcMessage is used to transport messages between tasks.
	StorageManagerIF* tcStore;
	StorageManagerIF* tmStore;
	object_id_t ccsdsPacketDistributor;
	bool communicationLinkUp; //!< Used to specify whether communication link is up
	bool tmStored;

	/**
	 * Handle TC reception. Default implementation provided
	 * @return
	 */
	virtual ReturnValue_t handleTc();

	/**
	 * Implemented by child class. Perform receiving of Telecommand
	 * @return
	 */
	virtual ReturnValue_t receiveTc() = 0;

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
	virtual ReturnValue_t readTmQueue();

	/**
	 * Implemented by child class. Perform sending of Telemetry
	 * @param data
	 * @param dataLen
	 * @return
	 */
	virtual ReturnValue_t sendTm(const uint8_t * data, uint32_t dataLen) = 0;

	/**
	 * Store data to be sent later if communication link is not up.
	 * @param message
	 * @return
	 */
	ReturnValue_t storeDownlinkData(TmTcMessage * message);

	/**
	 * Send stored data if communication link is active
	 * @return
	 */
	ReturnValue_t sendStoredTm();

	/**
	 * Print data as hexidecimal array
	 * @param data
	 * @param dataLen
	 */
	void printData(uint8_t * data, uint32_t dataLen);

	void registerCommConnect();
	void registerCommDisconnect();

private:
	static const uint8_t TMTC_RECEPTION_QUEUE_DEPTH = 20;
	static const uint8_t MAX_STORED_DATA_SENT_PER_CYCLE = 10;
	static const uint8_t MAX_DOWNLINK_PACKETS_STORED = 20;

	FIFO<store_address_t, MAX_DOWNLINK_PACKETS_STORED> fifo;
};


#endif /* FRAMEWORK_TMTCSERVICES_TMTCBRIDGE_H_ */
