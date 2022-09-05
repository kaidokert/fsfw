#include "DestHandler.h"

#include <etl/crc32.h>

#include <utility>

#include "fsfw/FSFW.h"
#include "fsfw/cfdp/pdu/EofPduReader.h"
#include "fsfw/cfdp/pdu/FileDataReader.h"
#include "fsfw/cfdp/pdu/FinishedPduCreator.h"
#include "fsfw/cfdp/pdu/HeaderReader.h"
#include "fsfw/objectmanager.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

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
        // Store data was deleted in PDU handler because a store guard is used
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
    if (step == TransactionStep::RECEIVING_FILE_DATA_PDUS) {
      for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
        if (infoIter->pduType == PduType::FILE_DATA) {
          result = handleFileDataPdu(*infoIter);
          if (result != OK) {
            status = result;
          }
          // Store data was deleted in PDU handler because a store guard is used
          dp.packetListRef.erase(infoIter++);
        }
        // TODO: Support for check timer missing
        if (infoIter->pduType == PduType::FILE_DIRECTIVE and
            infoIter->directiveType == FileDirectives::EOF_DIRECTIVE) {
          result = handleEofPdu(*infoIter);
          if (result != OK) {
            status = result;
          }
          // Store data was deleted in PDU handler because a store guard is used
          dp.packetListRef.erase(infoIter++);
        }
      }
    }
    if (step == TransactionStep::TRANSFER_COMPLETION) {
      result = handleTransferCompletion();
      if (result != OK) {
        status = result;
      }
    }
    if (step == TransactionStep::SENDING_FINISHED_PDU) {
      result = sendFinishedPdu();
      if (result != OK) {
        status = result;
      }
      finish();
    }
    return status;
  }
  if (cfdpState == CfdpStates::BUSY_CLASS_2_ACKED) {
    // TODO: Will be implemented at a later stage
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "CFDP state machine for acknowledged mode not implemented yet" << std::endl;
#endif
  }
  return OK;
}

ReturnValue_t cfdp::DestHandler::passPacket(PacketInfo packet) {
  if (dp.packetListRef.full()) {
    return FAILED;
  }
  dp.packetListRef.push_back(packet);
  return OK;
}

ReturnValue_t cfdp::DestHandler::initialize() {
  if (fp.tmStore == nullptr) {
    fp.tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (fp.tmStore == nullptr) {
      return FAILED;
    }
  }

  if (fp.tcStore == nullptr) {
    fp.tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (fp.tcStore == nullptr) {
      return FAILED;
    }
  }
  return OK;
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
    return constAccessorPair.first;
  }
  cfdp::FileSize offset;
  FileDataInfo fdInfo(offset);
  FileDataReader reader(constAccessorPair.second.data(), constAccessorPair.second.size(), fdInfo);
  ReturnValue_t result = reader.parseData();
  if (result != OK) {
    return result;
  }
  size_t fileSegmentLen = 0;
  const uint8_t* fileData = fdInfo.getFileData(&fileSegmentLen);
  FileOpParams fileOpParams(tp.sourceName.data(), fileSegmentLen);
  if (dp.cfg.indicCfg.fileSegmentRecvIndicRequired) {
    FileSegmentRecvdParams segParams;
    segParams.offset = offset.value();
    segParams.id = tp.transactionId;
    segParams.length = fileSegmentLen;
    segParams.recContState = fdInfo.getRecordContinuationState();
    size_t segmentMetadatLen = 0;
    auto* segMetadata = fdInfo.getSegmentMetadata(&segmentMetadatLen);
    segParams.segmentMetadata = {segMetadata, segmentMetadatLen};
    dp.user.fileSegmentRecvdIndication(segParams);
  }
  result = dp.user.vfs.writeToFile(fileOpParams, fileData);
  if (offset.value() + fileSegmentLen > tp.progress) {
    tp.progress = offset.value() + fileSegmentLen;
  }
  if (result != returnvalue::OK) {
    // TODO: Proper Error handling
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "File write error" << std::endl;
#endif
  } else {
    tp.deliveryStatus = FileDeliveryStatus::RETAINED_IN_FILESTORE;
  }
  return result;
}

ReturnValue_t cfdp::DestHandler::handleEofPdu(const cfdp::PacketInfo& info) {
  // Process EOF PDU
  auto constAccessorPair = fp.tcStore->getData(info.storeId);
  if (constAccessorPair.first != OK) {
    // TODO: This is not a CFDP error. Event and/or warning?
    return constAccessorPair.first;
  }
  EofInfo eofInfo(nullptr);
  EofPduReader reader(constAccessorPair.second.data(), constAccessorPair.second.size(), eofInfo);
  ReturnValue_t result = reader.parseData();
  if (result != OK) {
    return result;
  }
  // TODO: Error handling
  if (eofInfo.getConditionCode() == ConditionCode::NO_ERROR) {
    tp.crc = eofInfo.getChecksum();
    uint64_t fileSizeFromEof = eofInfo.getFileSize().value();
    // CFDP 4.6.1.2.9: Declare file size error if progress exceeds file size
    if (fileSizeFromEof > tp.progress) {
      // TODO: File size error
    }
    tp.fileSize.setFileSize(fileSizeFromEof, std::nullopt);
  }
  if (step == TransactionStep::RECEIVING_FILE_DATA_PDUS) {
    if (cfdpState == CfdpStates::BUSY_CLASS_1_NACKED) {
      step = TransactionStep::TRANSFER_COMPLETION;
    } else if (cfdpState == CfdpStates::BUSY_CLASS_2_ACKED) {
      step = TransactionStep::SENDING_ACK_PDU;
    }
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

ReturnValue_t cfdp::DestHandler::handleTransferCompletion() {
  ReturnValue_t result;
  if (tp.checksumType != ChecksumTypes::NULL_CHECKSUM) {
    result = checksumVerification();
    if (result != OK) {
      // TODO: Warning / error handling?
    }
  } else {
    tp.conditionCode = ConditionCode::NO_ERROR;
  }
  result = noticeOfCompletion();
  if (result != OK) {
  }
  if (cfdpState == CfdpStates::BUSY_CLASS_1_NACKED) {
    if (tp.closureRequested) {
      step = TransactionStep::SENDING_FINISHED_PDU;
    } else {
      finish();
    }
  } else if (cfdpState == CfdpStates::BUSY_CLASS_2_ACKED) {
    step = TransactionStep::SENDING_FINISHED_PDU;
  }
  return OK;
}

void cfdp::DestHandler::finish() {
  tp.reset();
  dp.packetListRef.clear();
  cfdpState = CfdpStates::IDLE;
  step = TransactionStep::IDLE;
}

ReturnValue_t cfdp::DestHandler::checksumVerification() {
  std::array<uint8_t, 1024> buf{};
  // TODO: Checksum verification and notice of completion
  etl::crc32 crcCalc;
  uint64_t currentOffset = 0;
  FileOpParams params(tp.sourceName.data(), buf.size());
  while (currentOffset < tp.fileSize.value()) {
    uint64_t readLen = 0;
    if (currentOffset + buf.size() > tp.fileSize.value()) {
      readLen = tp.fileSize.value() - currentOffset;
    } else {
      readLen = buf.size();
    }
    if (readLen > 0) {
      params.offset = currentOffset;
      params.size = readLen;
      auto result = dp.user.vfs.readFromFile(params, buf.data(), buf.size());
      if (result != OK) {
        // TODO: Better error handling
        return FAILED;
      }
      crcCalc.add(buf.begin(), buf.begin() + readLen);
    }
    currentOffset += readLen;
  }

  uint32_t value = crcCalc.value();
  if (value == tp.crc) {
    tp.conditionCode = ConditionCode::NO_ERROR;
    tp.deliveryCode = FileDeliveryCode::DATA_COMPLETE;
  } else {
    // TODO: Proper error handling
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CRC check for file " << tp.sourceName.data() << " failed" << std::endl;
#endif
    tp.conditionCode = ConditionCode::FILE_CHECKSUM_FAILURE;
  }
  return OK;
}

ReturnValue_t cfdp::DestHandler::noticeOfCompletion() {
  if (dp.cfg.indicCfg.transactionFinishedIndicRequired) {
    TransactionFinishedParams params(tp.transactionId, tp.conditionCode, tp.deliveryCode,
                                     tp.deliveryStatus);
    dp.user.transactionFinishedIndication(params);
  }
  return OK;
}

ReturnValue_t cfdp::DestHandler::sendFinishedPdu() {
  FinishedInfo info(tp.conditionCode, tp.deliveryCode, tp.deliveryStatus);
  FinishPduCreator finishedPdu(tp.pduConf, info);
  store_address_t storeId;
  uint8_t* dataPtr = nullptr;
  ReturnValue_t result =
      fp.tcStore->getFreeElement(&storeId, finishedPdu.getSerializedSize(), &dataPtr);
  if (result != OK) {
    // TODO: Error handling and event, this is a non CFDP specific error (most likely store is full)
    return result;
  }
  size_t serLen = 0;
  result = finishedPdu.serialize(dataPtr, serLen, finishedPdu.getSerializedSize());
  if (result != OK) {
    // TODO: Error printout, this really should not happen
    return result;
  }
  TmTcMessage msg(storeId);
  result = fp.msgQueue.sendMessage(fp.packetDest.getReportReceptionQueue(), &msg);
  if (result != OK) {
    // TODO: Error handling and event, this is a non CFDP specific error (most likely store is full)
    return result;
  }
  return OK;
}

cfdp::DestHandler::TransactionStep cfdp::DestHandler::getTransactionStep() const { return step; }
