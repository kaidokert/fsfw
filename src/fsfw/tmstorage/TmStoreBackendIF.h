#ifndef FSFW_TMTCSERVICES_TMSTOREBACKENDIF_H_
#define FSFW_TMTCSERVICES_TMSTOREBACKENDIF_H_

#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/parameters/HasParametersIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/timemanager/Clock.h"
#include "tmStorageConf.h"

class TmPacketInformation;
class TmPacketMinimal;
class SpacePacketBase;
class ApidSsc;

class TmStoreBackendIF : public HasParametersIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::TM_STORE_BACKEND_IF;
  static const ReturnValue_t BUSY = MAKE_RETURN_CODE(1);
  static const ReturnValue_t FULL = MAKE_RETURN_CODE(2);
  static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(3);
  static const ReturnValue_t NULL_REQUESTED = MAKE_RETURN_CODE(4);
  static const ReturnValue_t TOO_LARGE = MAKE_RETURN_CODE(5);
  static const ReturnValue_t NOT_READY = MAKE_RETURN_CODE(6);
  static const ReturnValue_t DUMP_ERROR = MAKE_RETURN_CODE(7);
  static const ReturnValue_t CRC_ERROR = MAKE_RETURN_CODE(8);
  static const ReturnValue_t TIMEOUT = MAKE_RETURN_CODE(9);
  static const ReturnValue_t IDLE_PACKET_FOUND = MAKE_RETURN_CODE(10);
  static const ReturnValue_t TELECOMMAND_FOUND = MAKE_RETURN_CODE(11);
  static const ReturnValue_t NO_PUS_A_TM = MAKE_RETURN_CODE(12);
  static const ReturnValue_t TOO_SMALL = MAKE_RETURN_CODE(13);
  static const ReturnValue_t BLOCK_NOT_FOUND = MAKE_RETURN_CODE(14);
  static const ReturnValue_t INVALID_REQUEST = MAKE_RETURN_CODE(15);

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::MEMORY;
  static const Event STORE_SEND_WRITE_FAILED =
      MAKE_EVENT(0, severity::LOW);  //!< Initiating sending data to store failed. Low, par1:
                                     //!< returnCode, par2: integer (debug info)
  static const Event STORE_WRITE_FAILED = MAKE_EVENT(
      1, severity::LOW);  //!< Data was sent, but writing failed. Low, par1: returnCode, par2: 0
  static const Event STORE_SEND_READ_FAILED =
      MAKE_EVENT(2, severity::LOW);  //!< Initiating reading data from store failed. Low, par1:
                                     //!< returnCode, par2: 0
  static const Event STORE_READ_FAILED = MAKE_EVENT(
      3, severity::LOW);  //!< Data was requested, but access failed. Low, par1: returnCode, par2: 0
  static const Event UNEXPECTED_MSG =
      MAKE_EVENT(4, severity::LOW);  //!< An unexpected TM packet or data message occurred. Low,
                                     //!< par1: 0, par2: integer (debug info)
  static const Event STORING_FAILED = MAKE_EVENT(
      5, severity::LOW);  //!< Storing data failed. May simply be a full store. Low, par1:
                          //!< returnCode, par2: integer (sequence count of failed packet).
  static const Event TM_DUMP_FAILED =
      MAKE_EVENT(6, severity::LOW);  //!< Dumping retrieved data failed. Low, par1: returnCode,
                                     //!< par2: integer (sequence count of failed packet).
  static const Event STORE_INIT_FAILED =
      MAKE_EVENT(7, severity::LOW);  //!< Corrupted init data or read error. Low, par1: returnCode,
                                     //!< par2: integer (debug info)
  static const Event STORE_INIT_EMPTY = MAKE_EVENT(
      8, severity::INFO);  //!< Store was not initialized. Starts empty. Info, parameters both zero.
  static const Event STORE_CONTENT_CORRUPTED =
      MAKE_EVENT(9, severity::LOW);  //!< Data was read out, but it is inconsistent. Low par1:
                                     //!< Memory address of corruption, par2: integer (debug info)
  static const Event STORE_INITIALIZE =
      MAKE_EVENT(10, severity::INFO);  //!< Info event indicating the store will be initialized,
                                       //!< either at boot or after IOB switch. Info. pars: 0
  static const Event INIT_DONE = MAKE_EVENT(
      11, severity::INFO);  //!< Info event indicating the store was successfully initialized,
                            //!< either at boot or after IOB switch. Info. pars: 0
  static const Event DUMP_FINISHED = MAKE_EVENT(
      12, severity::INFO);  //!< Info event indicating that dumping finished successfully. par1:
                            //!< Number of dumped packets. par2: APID/SSC (16bits each)
  static const Event DELETION_FINISHED = MAKE_EVENT(
      13, severity::INFO);  //!< Info event indicating that deletion finished successfully. par1:
                            //!< Number of deleted packets. par2: APID/SSC (16bits each)
  static const Event DELETION_FAILED = MAKE_EVENT(
      14,
      severity::LOW);  //!< Info event indicating that something went wrong during deletion. pars: 0
  static const Event AUTO_CATALOGS_SENDING_FAILED =
      MAKE_EVENT(15, severity::INFO);  //!< Info that the a auto catalog report failed

  virtual ~TmStoreBackendIF() {}

  /**
   * SHOULDDO: Specification on what has to be implemented here
   * @param opCode
   * @return
   */
  virtual ReturnValue_t performOperation(uint8_t opCode) = 0;
  virtual ReturnValue_t initialize() = 0;

  /**
   * Implement the storage of TM packets to mass memory
   * @param tmPacket
   * @return
   */
  virtual ReturnValue_t storePacket(TmPacketMinimal* tmPacket) = 0;
  virtual ReturnValue_t setFetchLimitTime(const timeval* loverLimit, const timeval* upperLimit) = 0;
  virtual ReturnValue_t setFetchLimitBlocks(uint32_t startAddress, uint32_t endAddress) = 0;
  virtual ReturnValue_t fetchPackets(bool fromBegin = false) = 0;
  virtual ReturnValue_t initializeStore(object_id_t dumpTarget) = 0;
  virtual ReturnValue_t dumpIndex(store_address_t* storeId) = 0;

  /**
   * SHOULDDO: Adapt for file management system?
   * @param startAddress
   * @param endAddress
   * @return
   */
  virtual ReturnValue_t deleteBlocks(uint32_t startAddress, uint32_t endAddress) = 0;
  virtual ReturnValue_t deleteTime(const timeval* timeUntil, uint32_t* deletedPackets) = 0;
  virtual void resetStore(bool clearStore, bool resetWrite, bool resetRead) = 0;
  virtual bool isReady() = 0;
  virtual uint32_t availableData() = 0;
  virtual float getPercentageFilled() const = 0;
  virtual uint32_t getStoredPacketsCount() const = 0;
  virtual TmPacketInformation* getOldestPacket() = 0;
  virtual TmPacketInformation* getYoungestPacket() = 0;
  virtual float getDataRate() = 0;
};

#endif /* FSFW_TMTCSERVICES_TMSTOREBACKENDIF_H_ */
