#ifndef FSFW_EXAMPLE_HOSTED_CFDPHANDLER_H
#define FSFW_EXAMPLE_HOSTED_CFDPHANDLER_H

#include <utility>

#include "fsfw/cfdp/handler/DestHandler.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

struct FsfwHandlerParams {
  FsfwHandlerParams(object_id_t objectId, HasFileSystemIF& vfs, AcceptsTelemetryIF& packetDest,
                    StorageManagerIF& tcStore, StorageManagerIF& tmStore)
      : objectId(objectId), vfs(vfs), packetDest(packetDest), tcStore(tcStore), tmStore(tmStore) {}
  object_id_t objectId{};
  HasFileSystemIF& vfs;
  AcceptsTelemetryIF& packetDest;
  StorageManagerIF& tcStore;
  StorageManagerIF& tmStore;
};

struct CfdpHandlerCfg {
  CfdpHandlerCfg(cfdp::EntityId localId, cfdp::IndicationCfg indicationCfg,
                 cfdp::UserBase& userHandler, cfdp::FaultHandlerBase& userFaultHandler,
                 cfdp::PacketInfoListBase& packetInfo, cfdp::LostSegmentsListBase& lostSegmentsList,
                 cfdp::RemoteConfigTableIF& remoteCfgProvider)
      : id(std::move(localId)),
        indicCfg(indicationCfg),
        packetInfoList(packetInfo),
        lostSegmentsList(lostSegmentsList),
        remoteCfgProvider(remoteCfgProvider),
        userHandler(userHandler),
        faultHandler(userFaultHandler) {}

  cfdp::EntityId id;
  cfdp::IndicationCfg indicCfg;
  cfdp::PacketInfoListBase& packetInfoList;
  cfdp::LostSegmentsListBase& lostSegmentsList;
  cfdp::RemoteConfigTableIF& remoteCfgProvider;
  cfdp::UserBase& userHandler;
  cfdp::FaultHandlerBase& faultHandler;
};

class CfdpHandler : public SystemObject, public ExecutableObjectIF, public AcceptsTelecommandsIF {
 public:
  explicit CfdpHandler(const FsfwHandlerParams& fsfwParams, const CfdpHandlerCfg& cfdpCfg);

  [[nodiscard]] const char* getName() const override;
  [[nodiscard]] uint32_t getIdentifier() const override;
  [[nodiscard]] MessageQueueId_t getRequestQueue() const override;

  ReturnValue_t initialize() override;
  ReturnValue_t performOperation(uint8_t operationCode) override;

 private:
  MessageQueueIF* msgQueue = nullptr;
  cfdp::DestHandler destHandler;
  StorageManagerIF* tcStore = nullptr;
  StorageManagerIF* tmStore = nullptr;

  ReturnValue_t handleCfdpPacket(TmTcMessage& msg);
};

#endif  // FSFW_EXAMPLE_HOSTED_CFDPHANDLER_H
