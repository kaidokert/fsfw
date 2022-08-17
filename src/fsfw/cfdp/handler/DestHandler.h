#ifndef FSFW_CFDP_CFDPDESTHANDLER_H
#define FSFW_CFDP_CFDPDESTHANDLER_H

#include <etl/list.h>

#include <utility>

#include "RemoteConfigTableIF.h"
#include "UserBase.h"
#include "fsfw/cfdp/handler/mib.h"
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
                    AcceptsTelemetryIF& packetDest, MessageQueueIF& msgQueue,
                    etl::ilist<PacketInfo>& packetList)
      : cfg(std::move(cfg)),
        user(user),
        remoteCfgTable(remoteCfgTable),
        packetDest(packetDest),
        msgQueue(msgQueue),
        packetListRef(packetList) {}

  LocalEntityCfg cfg;
  UserBase& user;
  RemoteConfigTableIF& remoteCfgTable;
  AcceptsTelemetryIF& packetDest;
  MessageQueueIF& msgQueue;
  StorageManagerIF* tcStore = nullptr;
  StorageManagerIF* tmStore = nullptr;
  etl::ilist<PacketInfo>& packetListRef;
};

class DestHandler {
 public:
  explicit DestHandler(DestHandlerParams params);

  ReturnValue_t performStateMachine();

  ReturnValue_t passPacket(PacketInfo packet);

  ReturnValue_t initialize();

 private:
  DestHandlerParams p;
  enum class TransactionStep {
    IDLE = 0,
    TRANSACTION_START = 1,
    RECEIVING_FILE_DATA_PDUS = 2,
    SENDING_ACK_PDU = 3,
    TRANSFER_COMPLETION = 4,
    SENDING_FINISHED_PDU = 5
  };
  TransactionStep step = TransactionStep::IDLE;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_CFDPDESTHANDLER_H
