#ifndef FSFW_CFDP_CFDPDESTHANDLER_H
#define FSFW_CFDP_CFDPDESTHANDLER_H

#include <etl/list.h>

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
  PacketInfo(PduType type, FileDirectives directive, store_address_t storeId)
      : pduType(type), directiveType(directive), storeId(storeId) {}

  PduType pduType = PduType::FILE_DATA;
  FileDirectives directiveType = FileDirectives::INVALID_DIRECTIVE;
  store_address_t storeId = store_address_t::invalid();
  PacketInfo() = default;
};

struct DestHandlerParams {
  DestHandlerParams(LocalEntityCfg cfg, UserBase& user, RemoteConfigTableIF& remoteCfgTable,
                    etl::ilist<PacketInfo>& packetList)
      : cfg(std::move(cfg)),
        user(user),
        remoteCfgTable(remoteCfgTable),
        packetListRef(packetList) {}

  LocalEntityCfg cfg;
  UserBase& user;
  RemoteConfigTableIF& remoteCfgTable;

  etl::ilist<PacketInfo>& packetListRef;
  uint8_t maxTlvsInOnePdu = 10;
  size_t maxFilenameLen = 255;
};

struct FsfwParams {
  FsfwParams(AcceptsTelemetryIF& packetDest, MessageQueueIF& msgQueue,
             EventReportingProxyIF& eventReporter)
      : packetDest(packetDest), msgQueue(msgQueue), eventReporter(eventReporter) {}
  AcceptsTelemetryIF& packetDest;
  MessageQueueIF& msgQueue;
  EventReportingProxyIF& eventReporter;
  StorageManagerIF* tcStore = nullptr;
  StorageManagerIF* tmStore = nullptr;
};

class DestHandler {
 public:
  /**
   * Will be returned if it is advisable to call the state machine operation call again
   */
  ReturnValue_t CALL_FSM_AGAIN = returnvalue::makeCode(1, 0);
  explicit DestHandler(DestHandlerParams handlerParams, FsfwParams fsfwParams);

  /**
   *
   * @return
   *  - @c returnvalue::OK  State machine OK for this execution cycle
   *  - @c CALL_FSM_AGAIN   State machine should be called again.
   */
  ReturnValue_t performStateMachine();

  ReturnValue_t passPacket(PacketInfo packet);

  ReturnValue_t initialize();

  ReturnValue_t handleMetadataPdu(const PacketInfo& info);
  ReturnValue_t handleFileDataPdu(const PacketInfo& info);
  ReturnValue_t handleMetadataParseError(const uint8_t* rawData, size_t maxSize);

  [[nodiscard]] CfdpStates getCfdpState() const;

 private:
  enum class TransactionStep {
    IDLE = 0,
    TRANSACTION_START = 1,
    RECEIVING_FILE_DATA_PDUS = 2,
    SENDING_ACK_PDU = 3,
    TRANSFER_COMPLETION = 4,
    SENDING_FINISHED_PDU = 5
  };
  struct TransactionParams {
    explicit TransactionParams(size_t maxFileNameLen)
        : sourceName(maxFileNameLen), destName(maxFileNameLen) {}

    ChecksumType checksumType = ChecksumType::NULL_CHECKSUM;
    bool closureRequested{};
    std::vector<char> sourceName;
    std::vector<char> destName;
    cfdp::FileSize fileSize;
    TransactionId transactionId;
    PduConfig pduConf;
    RemoteEntityCfg* remoteCfg = nullptr;
  };

  TransactionStep step = TransactionStep::IDLE;
  CfdpStates cfdpState = CfdpStates::IDLE;
  std::vector<cfdp::Tlv> tlvVec;
  std::vector<cfdp::Tlv> userTlvVec;
  DestHandlerParams dp;
  FsfwParams fp;
  TransactionParams tp;

  ReturnValue_t startTransaction(MetadataPduReader& reader, MetadataInfo& info);
};

}  // namespace cfdp

#endif  // FSFW_CFDP_CFDPDESTHANDLER_H
