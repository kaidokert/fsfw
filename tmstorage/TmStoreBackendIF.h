/*
 * TmStoreBackendIF.h
 *
 *  Created on: 18.02.2015
 *      Author: baetz
 */

#ifndef PLATFORM_TMTCSERVICES_TMSTOREBACKENDIF_H_
#define PLATFORM_TMTCSERVICES_TMSTOREBACKENDIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
class TmPacketInformation;
class TmPacketMinimal;

class TmStoreBackendIF {
public:
	static const uint8_t INTERFACE_ID = TM_STORE_BACKEND_IF;
	static const ReturnValue_t BUSY = MAKE_RETURN_CODE(1);
	static const ReturnValue_t FULL = MAKE_RETURN_CODE(2);
	static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(3);
	static const ReturnValue_t NULL_REQUESTED = MAKE_RETURN_CODE(4);
	static const ReturnValue_t TOO_LARGE = MAKE_RETURN_CODE(5);
	static const ReturnValue_t NOT_READY = MAKE_RETURN_CODE(6);
	static const ReturnValue_t DUMP_ERROR = MAKE_RETURN_CODE(7);
	static const ReturnValue_t CRC_ERROR = MAKE_RETURN_CODE(8);
	static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(9);

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::MEMORY;
	static const Event STORE_SEND_WRITE_FAILED = MAKE_EVENT(0, SEVERITY::LOW); //!< Initiating sending data to store failed. Low, par1: returnCode, par2: integer (debug info)
	static const Event STORE_WRITE_FAILED = MAKE_EVENT(1, SEVERITY::LOW); //!< Data was sent, but writing failed. Low, par1: returnCode, par2: 0
	static const Event STORE_SEND_READ_FAILED = MAKE_EVENT(2, SEVERITY::LOW); //!< Initiating reading data from store failed. Low, par1: returnCode, par2: 0
	static const Event STORE_READ_FAILED = MAKE_EVENT(3, SEVERITY::LOW); //!< Data was requested, but access failed. Low, par1: returnCode, par2: 0
	static const Event UNEXPECTED_MSG = MAKE_EVENT(4, SEVERITY::INFO); //!< An unexpected TM packet or data message occurred. Info, par1: 0, par2: integer (debug info)
	static const Event STORING_FAILED = MAKE_EVENT(5, SEVERITY::LOW); //!< Storing data failed. May simply be a full store. Low, par1: returnCode, par2: integer (sequence count of failed packet).
	static const Event TM_DUMP_FAILED = MAKE_EVENT(6, SEVERITY::LOW); //!< Dumping retrieved data failed. Low, par1: returnCode, par2: integer (sequence count of failed packet).
	static const Event STORE_INIT_FAILED = MAKE_EVENT(7, SEVERITY::LOW); //!< Corrupted init data or read error. Low, par1: returnCode, par2: integer (debug info)
	static const Event STORE_INIT_EMPTY = MAKE_EVENT(8, SEVERITY::INFO); //!< Store was not initialized. Starts empty. Info, parameters both zero.
	static const Event STORE_CONTENT_CORRUPTED = MAKE_EVENT(9, SEVERITY::LOW); //!< Data was read out, but it is inconsistent. Low par1: 0, par2: integer (debug info)
	static const Event STORE_INITIALIZE = MAKE_EVENT(10, SEVERITY::INFO); //!< Info event indicating the store will be initialized, either at boot or after IOB switch. Info. pars: 0
	static const Event STORE_INIT_DONE = MAKE_EVENT(11, SEVERITY::INFO); //!< Info event indicating the store was successfully initialized, either at boot or after IOB switch. Info. pars: 0

	virtual ~TmStoreBackendIF() {}
	virtual ReturnValue_t performOperation() = 0;
	virtual ReturnValue_t initialize() = 0;
	virtual ReturnValue_t storePacket(TmPacketMinimal* tmPacket) = 0;
	virtual ReturnValue_t fetchPackets(bool useAddress = false, uint32_t startAtAddress = 0) = 0;
	virtual ReturnValue_t deleteContent(bool deletePart = false, uint32_t upToAddress = 0, uint32_t nDeletedPackets = 0, TmPacketMinimal* newOldestPacket = NULL) = 0;
	virtual ReturnValue_t initializeStore() = 0;
	virtual void resetStore(bool resetWrite = false, bool resetRead = false) = 0;
	virtual bool isReady() = 0;
	virtual uint32_t availableData() = 0;
	virtual float getPercentageFilled() const = 0;
	virtual uint32_t getStoredPacketsCount() const = 0;
	virtual TmPacketInformation* getOldestPacket() = 0;
	virtual TmPacketInformation* getYoungestPacket()  = 0;
};



#endif /* PLATFORM_TMTCSERVICES_TMSTOREBACKENDIF_H_ */
