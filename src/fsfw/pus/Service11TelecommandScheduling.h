#ifndef MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_
#define MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_

#include <etl/multimap.h>
#include <fsfw/tmtcservices/PusServiceBase.h>
#include <fsfw/tmtcservices/TmTcMessage.h>

#include "fsfw/FSFW.h"
#include "fsfw/returnvalues/FwClassIds.h"

/**
 * @brief: PUS-Service 11 - Telecommand scheduling.
 * @details:
 * PUS-Service 11 - Telecommand scheduling.
 * Full documentation: ECSS-E-ST-70-41C, p. 168:
 * ST[11] time-based scheduling
 *
 * This service provides the capability to command pre-loaded
 * application processes (telecommands) by releasing them at their
 * due-time.
 * References to telecommands are stored together with their due-timepoints
 * and are released at their corresponding due-time.
 *
 * Necessary subservice functionalities are implemented.
 * Those are:
 * TC[11,4] activity insertion
 * TC[11,5] activity deletion
 * TC[11,6] filter-based activity deletion
 * TC[11,7] activity time-shift
 * TC[11,8] filter-based activity time-shift
 *
 * Groups are not supported.
 * This service remains always enabled. Sending a disable-request has no effect.
 */
template <size_t MAX_NUM_TCS>
class Service11TelecommandScheduling final : public PusServiceBase {
 public:
  static constexpr uint8_t CLASS_ID = CLASS_ID::PUS_SERVICE_11;

  static constexpr ReturnValue_t INVALID_TYPE_TIME_WINDOW =
      HasReturnvaluesIF::makeReturnCode(CLASS_ID, 1);
  static constexpr ReturnValue_t TIMESHIFTING_NOT_POSSIBLE =
      HasReturnvaluesIF::makeReturnCode(CLASS_ID, 2);
  static constexpr ReturnValue_t INVALID_RELATIVE_TIME =
      HasReturnvaluesIF::makeReturnCode(CLASS_ID, 3);

  // The types of PUS-11 subservices
  enum Subservice : uint8_t {
    ENABLE_SCHEDULING = 1,
    DISABLE_SCHEDULING = 2,
    RESET_SCHEDULING = 3,
    INSERT_ACTIVITY = 4,
    DELETE_ACTIVITY = 5,
    FILTER_DELETE_ACTIVITY = 6,
    TIMESHIFT_ACTIVITY = 7,
    FILTER_TIMESHIFT_ACTIVITY = 8,
    DETAIL_REPORT = 9,
    TIMEBASE_SCHEDULE_DETAIL_REPORT = 10,
    TIMESHIFT_ALL_SCHEDULE_ACTIVITIES = 15
  };

  // The types of time windows for TC[11,6] and TC[11,8], as defined in ECSS-E-ST-70-41C,
  // requirement 8.11.3c (p. 507)
  enum TypeOfTimeWindow : uint32_t {
    SELECT_ALL = 0,
    FROM_TIMETAG_TO_TIMETAG = 1,
    FROM_TIMETAG = 2,
    TO_TIMETAG = 3
  };

  Service11TelecommandScheduling(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                                 AcceptsTelecommandsIF* tcRecipient,
                                 uint16_t releaseTimeMarginSeconds = DEFAULT_RELEASE_TIME_MARGIN,
                                 bool debugMode = false);

  ~Service11TelecommandScheduling();

  /** PusServiceBase overrides */
  ReturnValue_t handleRequest(uint8_t subservice) override;
  ReturnValue_t performService() override;
  ReturnValue_t initialize() override;

 private:
  struct TelecommandStruct {
    uint64_t requestId;
    uint32_t seconds;
    store_address_t storeAddr;  // uint16
  };

  static constexpr uint16_t DEFAULT_RELEASE_TIME_MARGIN = 5;

  // minimum release time offset to insert into schedule
  const uint16_t RELEASE_TIME_MARGIN_SECONDS = 5;

  // the maximum amount of stored TCs is defined here
  static constexpr uint16_t MAX_STORED_TELECOMMANDS = 500;

  bool debugMode = false;
  StorageManagerIF* tcStore = nullptr;
  AcceptsTelecommandsIF* tcRecipient = nullptr;
  MessageQueueId_t recipientMsgQueueId = 0;

  /**
   * The telecommand map uses the exectution time as a Unix time stamp as
   * the key. This is mapped to a generic telecommand struct.
   */
  using TelecommandMap = etl::multimap<uint32_t, TelecommandStruct, MAX_NUM_TCS>;
  using TcMapIter = typename TelecommandMap::iterator;

  TelecommandMap telecommandMap;

  /**
   * @brief Logic to be performed on an incoming TC[11,4].
   * @return RETURN_OK if successful
   */
  ReturnValue_t doInsertActivity(const uint8_t* data, size_t size);

  /**
   * @brief Logic to be performed on an incoming TC[11,5].
   * @return RETURN_OK if successful
   */
  ReturnValue_t doDeleteActivity(const uint8_t* data, size_t size);

  /**
   * @brief Logic to be performed on an incoming TC[11,6].
   * @return RETURN_OK if successful
   */
  ReturnValue_t doFilterDeleteActivity(const uint8_t* data, size_t size);

  /**
   * @brief Logic to be performed on an incoming TC[11,7].
   * @return RETURN_OK if successful
   */
  ReturnValue_t doTimeshiftActivity(const uint8_t* data, size_t size);

  /**
   * @brief Logic to be performed on an incoming TC[11,8].
   * @return RETURN_OK if successful
   */
  ReturnValue_t doFilterTimeshiftActivity(const uint8_t* data, size_t size);

  /**
   * @brief Deserializes a generic type from a payload buffer by using the FSFW
   * SerializeAdapter Interface.
   * @param output Output to be deserialized
   * @param buf Payload buffer (application data)
   * @param bufsize Remaining size of payload buffer (application data size)
   * @return RETURN_OK if successful
   */
  template <typename T>
  ReturnValue_t deserializeViaFsfwInterface(T& output, const uint8_t* buf, size_t bufsize);

  /**
   * @brief Extracts the Request ID from the Application Data of a TC by utilizing a ctor of the
   * class TcPacketPus.
   * NOTE: This only works if the payload data is a TC (e.g. not for TC[11,5] which does not
   * send a TC as payload)!
   * @param data The Application data of the TC (get via getApplicationData()).
   * @return requestId
   */
  uint64_t getRequestIdFromDataTC(const uint8_t* data) const;

  /**
   * @brief Extracts the Request ID from the Application Data directly, assuming it is packed
   * as follows (acc. to ECSS): | source ID (uint32) | apid (uint32) | ssc (uint32) |.
   * @param data Pointer to first byte described data
   * @param dataSize Remaining size of data NOTE: non-const, this is modified by the function
   * @param [out] requestId Request ID
   * @return RETURN_OK if successful
   */
  ReturnValue_t getRequestIdFromData(const uint8_t*& data, size_t& dataSize, uint64_t& requestId);

  /**
   * @brief Builds the Request ID from its three elements.
   * @param sourceId Source ID
   * @param apid Application Process ID (APID)
   * @param ssc Source Sequence Count
   * @return Request ID
   */
  uint64_t buildRequestId(uint32_t sourceId, uint16_t apid, uint16_t ssc) const;

  /**
   * @brief Gets the filter range for filter TCs from a data packet
   * @param data TC data
   * @param dataSize TC data size
   * @param [out] itBegin Begin of filter range
   * @param [out] itEnd End of filter range
   * @return RETURN_OK if successful
   */
  ReturnValue_t getMapFilterFromData(const uint8_t*& data, size_t& size, TcMapIter& itBegin,
                                     TcMapIter& itEnd);

  ReturnValue_t handleInvalidData(const char* ctx);
  /**
   * @brief Prints content of multimap. Use for simple debugging only.
   */
  void debugPrintMultimapContent(void) const;
};

#include "Service11TelecommandScheduling.tpp"

#endif /* MISSION_PUS_SERVICE11TELECOMMANDSCHEDULING_H_ */
