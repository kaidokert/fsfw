#include "DestHandler.h"

#include <utility>

#include "fsfw/cfdp/pdu/FileDataReader.h"
#include "fsfw/cfdp/pdu/HeaderReader.h"
#include "fsfw/objectmanager.h"
#include "fsfw/serviceinterface.h"

using namespace returnvalue;

cfdp::DestHandler::DestHandler(DestHandlerParams params, FsfwParams fsfwParams)
    : tlvVec(params.maxTlvsInOnePdu),
      userTlvVec(params.maxTlvsInOnePdu),
      dp(std::move(params)),
      fp(fsfwParams),
      tp(params.maxFilenameLen) {
  tp.pduConf.direction = cfdp::Direction::TOWARDS_SENDER;
}

ReturnValue_t cfdp::DestHandler::performStateMachine() {
  ReturnValue_t result;
  ReturnValue_t status = returnvalue::OK;
  if (step == TransactionStep::IDLE) {
    for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
      if (infoIter->pduType == PduType::FILE_DIRECTIVE and
          infoIter->directiveType == FileDirectives::METADATA) {
        result = handleMetadataPdu(*infoIter);
        if (result != OK) {
          status = result;
        }
        // metadata packet was deleted in metadata handler because a store guard is used
        dp.packetListRef.erase(infoIter++);
      }
    }
    if (step == TransactionStep::IDLE) {
      // To decrease the already high complexity of the software, all packets arriving before
      // a metadata PDU are deleted.
      for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
        fp.tcStore->deleteData(infoIter->storeId);
      }
      dp.packetListRef.clear();
    }

    if (step != TransactionStep::IDLE) {
      return CALL_FSM_AGAIN;
    }
    return status;
  }
  if (cfdpState == CfdpStates::BUSY_CLASS_1_NACKED) {
    for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
      if (infoIter->pduType == PduType::FILE_DATA) {
        result = handleFileDataPdu(*infoIter);
        if (result != OK) {
          status = result;
        }
      }
    }
    return returnvalue::OK;
  }
  if (cfdpState == CfdpStates::BUSY_CLASS_2_ACKED) {
    // TODO: Will be implemented at a later stage
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "CFDP state machine for acknowledged mode not implemented yet" << std::endl;
#endif
  }
  return returnvalue::OK;
}

ReturnValue_t cfdp::DestHandler::passPacket(PacketInfo packet) {
  if (dp.packetListRef.full()) {
    return returnvalue::FAILED;
  }
  dp.packetListRef.push_back(packet);
  return returnvalue::OK;
}

ReturnValue_t cfdp::DestHandler::initialize() {
  if (fp.tmStore == nullptr) {
    fp.tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (fp.tmStore == nullptr) {
      return returnvalue::FAILED;
    }
  }

  if (fp.tcStore == nullptr) {
    fp.tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (fp.tcStore == nullptr) {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t cfdp::DestHandler::handleMetadataPdu(const PacketInfo& info) {
  // Process metadata PDU
  auto constAccessorPair = fp.tcStore->getData(info.storeId);
  if (constAccessorPair.first != OK) {
    // TODO: This is not a CFDP error. Event and/or warning?
    return constAccessorPair.first;
  }
  cfdp::StringLv sourceFileName;
  cfdp::StringLv destFileName;
  MetadataInfo metadataInfo(tp.fileSize, sourceFileName, destFileName);
  cfdp::Tlv* tlvArrayAsPtr = tlvVec.data();
  metadataInfo.setOptionsArray(&tlvArrayAsPtr, std::nullopt, tlvVec.size());
  MetadataPduReader reader(constAccessorPair.second.data(), constAccessorPair.second.size(),
                           metadataInfo);
  ReturnValue_t result = reader.parseData();
  // TODO: The standard does not really specify what happens if this kind of error happens
  //       I think it might be a good idea to cache some sort of error code, which
  //       is translated into a warning and/or event by an upper layer
  if (result != OK) {
    return handleMetadataParseError(constAccessorPair.second.data(),
                                    constAccessorPair.second.size());
  }
  return startTransaction(reader, metadataInfo);
}

ReturnValue_t cfdp::DestHandler::handleFileDataPdu(const cfdp::PacketInfo& info) {
  // Process file data PDU
  auto constAccessorPair = fp.tcStore->getData(info.storeId);
  if (constAccessorPair.first != OK) {
    // TODO: This is not a CFDP error. Event and/or warning?
    cfdp::FileSize offset;
    FileDataInfo fdInfo(offset);
    FileDataReader reader(constAccessorPair.second.data(), constAccessorPair.second.size(), fdInfo);
    return constAccessorPair.first;
  }
  return returnvalue::OK;
}

ReturnValue_t cfdp::DestHandler::handleMetadataParseError(const uint8_t* rawData, size_t maxSize) {
  // TODO: try to extract destination ID for error
  // TODO: Invalid metadata PDU.
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Parsing Metadata PDU failed with code " << result << std::endl;
#else
#endif
  HeaderReader headerReader(rawData, maxSize);
  ReturnValue_t result = headerReader.parseData();
  if (result != OK) {
    // TODO: Now this really should not happen. Warning or error,
    //       yield or cache appropriate returnvalue
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Parsing Header failed" << std::endl;
#else
#endif
    // TODO: Trigger appropriate event
    return result;
  }
  cfdp::EntityId destId;
  headerReader.getDestId(destId);
  RemoteEntityCfg* remoteCfg;
  if (not dp.remoteCfgTable.getRemoteCfg(destId, &remoteCfg)) {
// TODO: No remote config for dest ID. I consider this a configuration error.
//       Warning or error, yield or cache appropriate returnvalue
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "No remote config exists for destination ID" << std::endl;
#else
#endif
    // TODO: Trigger appropriate event
  }
  // TODO: Appropriate returnvalue?
  return returnvalue::FAILED;
}

ReturnValue_t cfdp::DestHandler::startTransaction(MetadataPduReader& reader, MetadataInfo& info) {
  if (cfdpState != CfdpStates::IDLE) {
    // According to standard, discard metadata PDU if we are busy
    return returnvalue::OK;
  }
  step = TransactionStep::TRANSACTION_START;
  if (reader.getTransmissionMode() == TransmissionModes::UNACKNOWLEDGED) {
    cfdpState = CfdpStates::BUSY_CLASS_1_NACKED;
  } else if (reader.getTransmissionMode() == TransmissionModes::ACKNOWLEDGED) {
    cfdpState = CfdpStates::BUSY_CLASS_2_ACKED;
  }
  tp.checksumType = info.getChecksumType();
  tp.closureRequested = info.isClosureRequested();
  size_t sourceNameSize = 0;
  const uint8_t* sourceNamePtr = info.getSourceFileName().getValue(&sourceNameSize);
  if (sourceNameSize > tp.sourceName.size()) {
    // TODO: Warning, event etc.
    return FAILED;
  }
  std::memcpy(tp.sourceName.data(), sourceNamePtr, sourceNameSize);
  tp.sourceName[sourceNameSize] = '\0';
  size_t destNameSize = 0;
  const uint8_t* destNamePtr = info.getDestFileName().getValue(&destNameSize);
  if (destNameSize > tp.destName.size()) {
    // TODO: Warning, event etc.
    return FAILED;
  }
  std::memcpy(tp.destName.data(), destNamePtr, destNameSize);
  tp.destName[destNameSize] = '\0';
  reader.fillConfig(tp.pduConf);
  tp.pduConf.direction = Direction::TOWARDS_SENDER;
  tp.transactionId.entityId = tp.pduConf.sourceId;
  tp.transactionId.seqNum = tp.pduConf.seqNum;
  if (not dp.remoteCfgTable.getRemoteCfg(tp.pduConf.destId, &tp.remoteCfg)) {
    // TODO: Warning, event etc.
    return FAILED;
  }
  step = TransactionStep::RECEIVING_FILE_DATA_PDUS;
  MetadataRecvdParams params(tp.transactionId, tp.pduConf.sourceId);
  params.fileSize = tp.fileSize.getSize();
  params.destFileName = tp.destName.data();
  params.sourceFileName = tp.sourceName.data();
  params.msgsToUserArray = dynamic_cast<MessageToUserTlv*>(userTlvVec.data());
  params.msgsToUserLen = info.getOptionsLen();
  dp.user.metadataRecvdIndication(params);
  return OK;
}

cfdp::CfdpStates cfdp::DestHandler::getCfdpState() const { return cfdpState; }
