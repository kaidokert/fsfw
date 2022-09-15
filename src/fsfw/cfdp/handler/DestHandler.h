#ifndef FSFW_CFDP_CFDPDESTHANDLER_H
#define FSFW_CFDP_CFDPDESTHANDLER_H

#include <etl/list.h>
#include <etl/set.h>

#include <optional>
#include <utility>

#include "RemoteConfigTableIF.h"
#include "UserBase.h"
#include "defs.h"
#include "fsfw/cfdp/handler/mib.h"
#include "fsfw/cfdp/pdu/MetadataPduReader.h"
#include "fsfw/cfdp/pdu/PduConfig.h"
#include "fsfw/container/DynamicFIFO.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/storagemanager/storeAddress.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

namespace cfdp {

struct PacketInfo {
  PacketInfo(PduType type, store_address_t storeId,
             std::optional<FileDirective> directive = std::nullopt)
      : pduType(type), directiveType(directive), storeId(storeId) {}

  PduType pduType = PduType::FILE_DATA;
  std::optional<FileDirective> directiveType = FileDirective::INVALID_DIRECTIVE;
  store_address_t storeId = store_address_t::invalid();
  PacketInfo() = default;
};

template <size_t SIZE>
using LostSegmentsList = etl::set<etl::pair<uint64_t, uint64_t>, SIZE>;
template <size_t SIZE>
using PacketInfoList = etl::list<PacketInfo, SIZE>;
using LostSegmentsListBase = etl::iset<etl::pair<uint64_t, uint64_t>>;
using PacketInfoListBase = etl::ilist<PacketInfo>;

struct DestHandlerParams {
  DestHandlerParams(LocalEntityCfg cfg, UserBase& user, RemoteConfigTableIF& remoteCfgTable,
                    PacketInfoListBase& packetList,
                    // TODO: This container can potentially take tons of space. For a better
                    //       memory efficient implementation, an additional abstraction could be
                    //       be used so users can use uint32_t as the pair type
                    LostSegmentsListBase& lostSegmentsContainer)
      : cfg(std::move(cfg)),
        user(user),
        remoteCfgTable(remoteCfgTable),
        packetListRef(packetList),
        lostSegmentsContainer(lostSegmentsContainer) {}

  LocalEntityCfg cfg;
  UserBase& user;
  RemoteConfigTableIF& remoteCfgTable;

  PacketInfoListBase& packetListRef;
  LostSegmentsListBase& lostSegmentsContainer;
  uint8_t maxTlvsInOnePdu = 10;
  size_t maxFilenameLen = 255;
};

struct FsfwParams {
  FsfwParams(AcceptsTelemetryIF& packetDest, MessageQueueIF* msgQueue,
             EventReportingProxyIF* eventReporter, StorageManagerIF& tcStore,
             StorageManagerIF& tmStore)
      : FsfwParams(packetDest, msgQueue, eventReporter) {
    this->tcStore = &tcStore;
    this->tmStore = &tmStore;
  }

  FsfwParams(AcceptsTelemetryIF& packetDest, MessageQueueIF* msgQueue,
             EventReportingProxyIF* eventReporter)
      : packetDest(packetDest), msgQueue(msgQueue), eventReporter(eventReporter) {}
  AcceptsTelemetryIF& packetDest;
  MessageQueueIF* msgQueue;
  EventReportingProxyIF* eventReporter = nullptr;
  StorageManagerIF* tcStore = nullptr;
  StorageManagerIF* tmStore = nullptr;
};

enum class CallStatus { DONE, CALL_AFTER_DELAY, CALL_AGAIN };

class DestHandler {
 public:
  enum class TransactionStep {
    IDLE = 0,
    TRANSACTION_START = 1,
    RECEIVING_FILE_DATA_PDUS = 2,
    SENDING_ACK_PDU = 3,
    TRANSFER_COMPLETION = 4,
    SENDING_FINISHED_PDU = 5
  };

  struct FsmResult {
   public:
    ReturnValue_t result = returnvalue::OK;
    CallStatus callStatus = CallStatus::CALL_AFTER_DELAY;
    TransactionStep step = TransactionStep::IDLE;
    CfdpStates state = CfdpStates::IDLE;
    uint32_t packetsSent = 0;
    uint8_t errors = 0;
    std::array<ReturnValue_t, 3> errorCodes = {};
    void resetOfIteration() {
      result = returnvalue::OK;
      callStatus = CallStatus::CALL_AFTER_DELAY;
      packetsSent = 0;
      errors = 0;
      errorCodes.fill(returnvalue::OK);
    }
  };
  /**
   * Will be returned if it is advisable to call the state machine operation call again
   */
  ReturnValue_t PARTIAL_SUCCESS = returnvalue::makeCode(0, 2);
  ReturnValue_t FAILURE = returnvalue::makeCode(0, 3);
  explicit DestHandler(DestHandlerParams handlerParams, FsfwParams fsfwParams);

  /**
   *
   * @return
   *  - @c returnvalue::OK  State machine OK for this execution cycle
   *  - @c CALL_FSM_AGAIN   State machine should be called again.
   */
  const FsmResult& performStateMachine();
  void setMsgQueue(MessageQueueIF& queue);
  void setEventReporter(EventReportingProxyIF& reporter);

  ReturnValue_t passPacket(PacketInfo packet);

  ReturnValue_t initialize();

  [[nodiscard]] CfdpStates getCfdpState() const;
  [[nodiscard]] TransactionStep getTransactionStep() const;
  [[nodiscard]] const TransactionId& getTransactionId() const;
  [[nodiscard]] const DestHandlerParams& getDestHandlerParams() const;
  [[nodiscard]] StorageManagerIF* getTcStore() const;
  [[nodiscard]] StorageManagerIF* getTmStore() const;

 private:
  struct TransactionParams {
    // Initialize char vectors with length + 1 for 0 termination
    explicit TransactionParams(size_t maxFileNameLen)
        : sourceName(maxFileNameLen + 1), destName(maxFileNameLen + 1) {}

    void reset() {
      pduConf = PduConfig();
      transactionId = TransactionId();
      std::fill(sourceName.begin(), sourceName.end(), '\0');
      std::fill(destName.begin(), destName.end(), '\0');
      fileSize.setFileSize(0, false);
      conditionCode = ConditionCode::NO_ERROR;
      deliveryCode = FileDeliveryCode::DATA_INCOMPLETE;
      deliveryStatus = FileDeliveryStatus::DISCARDED_DELIBERATELY;
      crc = 0;
      progress = 0;
      remoteCfg = nullptr;
      closureRequested = false;
      checksumType = ChecksumType::NULL_CHECKSUM;
    }

    ChecksumType checksumType = ChecksumType::NULL_CHECKSUM;
    bool closureRequested = false;
    std::vector<char> sourceName;
    std::vector<char> destName;
    cfdp::FileSize fileSize;
    TransactionId transactionId;
    PduConfig pduConf;
    ConditionCode conditionCode = ConditionCode::NO_ERROR;
    FileDeliveryCode deliveryCode = FileDeliveryCode::DATA_INCOMPLETE;
    FileDeliveryStatus deliveryStatus = FileDeliveryStatus::DISCARDED_DELIBERATELY;
    uint32_t crc = 0;
    uint64_t progress = 0;
    RemoteEntityCfg* remoteCfg = nullptr;
  };

  std::vector<cfdp::Tlv> tlvVec;
  std::vector<cfdp::Tlv> userTlvVec;
  DestHandlerParams dp;
  FsfwParams fp;
  TransactionParams tp;
  FsmResult fsmRes;

  ReturnValue_t startTransaction(MetadataPduReader& reader, MetadataInfo& info);
  ReturnValue_t handleMetadataPdu(const PacketInfo& info);
  ReturnValue_t handleFileDataPdu(const PacketInfo& info);
  ReturnValue_t handleEofPdu(const PacketInfo& info);
  ReturnValue_t handleMetadataParseError(ReturnValue_t result, const uint8_t* rawData,
                                         size_t maxSize);
  ReturnValue_t handleTransferCompletion();
  ReturnValue_t sendFinishedPdu();
  ReturnValue_t noticeOfCompletion();
  ReturnValue_t checksumVerification();
  const FsmResult& updateFsmRes(uint8_t errors);
  void checkAndHandleError(ReturnValue_t result, uint8_t& errorIdx);
  void finish();
};

}  // namespace cfdp

#endif  // FSFW_CFDP_CFDPDESTHANDLER_H
