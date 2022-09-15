#include "DestHandler.h"

#include <etl/crc32.h>

#include <utility>

#include "fsfw/FSFW.h"
#include "fsfw/cfdp/pdu/EofPduReader.h"
#include "fsfw/cfdp/pdu/FileDataReader.h"
#include "fsfw/cfdp/pdu/FinishedPduCreator.h"
#include "fsfw/cfdp/pdu/PduHeaderReader.h"
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

const cfdp::DestHandler::FsmResult& cfdp::DestHandler::performStateMachine() {
  ReturnValue_t result;
  uint8_t errorIdx = 0;
  fsmRes.resetOfIteration();
  if (fsmRes.step == TransactionStep::IDLE) {
    for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
      if (infoIter->pduType == PduType::FILE_DIRECTIVE and
          infoIter->directiveType == FileDirective::METADATA) {
        result = handleMetadataPdu(*infoIter);
        checkAndHandleError(result, errorIdx);
        // Store data was deleted in PDU handler because a store guard is used
        dp.packetListRef.erase(infoIter++);
      } else {
        infoIter++;
      }
    }
    if (fsmRes.step == TransactionStep::IDLE) {
      // To decrease the already high complexity of the software, all packets arriving before
      // a metadata PDU are deleted.
      for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
        fp.tcStore->deleteData(infoIter->storeId);
        infoIter++;
      }
      dp.packetListRef.clear();
    }

    if (fsmRes.step != TransactionStep::IDLE) {
      fsmRes.callStatus = CallStatus::CALL_AGAIN;
    }
    return updateFsmRes(errorIdx);
  }
  if (fsmRes.state == CfdpStates::BUSY_CLASS_1_NACKED) {
    if (fsmRes.step == TransactionStep::RECEIVING_FILE_DATA_PDUS) {
      for (auto infoIter = dp.packetListRef.begin(); infoIter != dp.packetListRef.end();) {
        if (infoIter->pduType == PduType::FILE_DATA) {
          result = handleFileDataPdu(*infoIter);
          checkAndHandleError(result, errorIdx);
          // Store data was deleted in PDU handler because a store guard is used
          dp.packetListRef.erase(infoIter++);
        } else if (infoIter->pduType == PduType::FILE_DIRECTIVE and
                   infoIter->directiveType == FileDirective::EOF_DIRECTIVE) {
          // TODO: Support for check timer missing
          result = handleEofPdu(*infoIter);
          checkAndHandleError(result, errorIdx);
          // Store data was deleted in PDU handler because a store guard is used
          dp.packetListRef.erase(infoIter++);
        } else {
          infoIter++;
        }
      }
    }
    if (fsmRes.step == TransactionStep::TRANSFER_COMPLETION) {
      result = handleTransferCompletion();
      checkAndHandleError(result, errorIdx);
    }
    if (fsmRes.step == TransactionStep::SENDING_FINISHED_PDU) {
      result = sendFinishedPdu();
      checkAndHandleError(result, errorIdx);
      finish();
    }
    return updateFsmRes(errorIdx);
  }
  if (fsmRes.state == CfdpStates::BUSY_CLASS_2_ACKED) {
    // TODO: Will be implemented at a later stage
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "CFDP state machine for acknowledged mode not implemented yet" << std::endl;
#endif
  }
  return updateFsmRes(errorIdx);
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

  if (fp.msgQueue == nullptr) {
    return FAILED;
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
    return handleMetadataParseError(result, constAccessorPair.second.data(),
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
  FileOpParams fileOpParams(tp.destName.data(), fileSegmentLen);
  fileOpParams.offset = offset.value();
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
  if (dp.cfg.indicCfg.eofRecvIndicRequired) {
    dp.user.eofRecvIndication(getTransactionId());
  }
  if (fsmRes.step == TransactionStep::RECEIVING_FILE_DATA_PDUS) {
    if (fsmRes.state == CfdpStates::BUSY_CLASS_1_NACKED) {
      fsmRes.step = TransactionStep::TRANSFER_COMPLETION;
    } else if (fsmRes.state == CfdpStates::BUSY_CLASS_2_ACKED) {
      fsmRes.step = TransactionStep::SENDING_ACK_PDU;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t cfdp::DestHandler::handleMetadataParseError(ReturnValue_t result,
                                                          const uint8_t* rawData, size_t maxSize) {
  // TODO: try to extract destination ID for error
  // TODO: Invalid metadata PDU.
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Parsing Metadata PDU failed with code " << result << std::endl;
#else
#endif
  PduHeaderReader headerReader(rawData, maxSize);
  result = headerReader.parseData();
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
// TODO: No remote config for dest ID. I consider this a configuration error, which is not
//       covered by the standard.
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
  if (fsmRes.state != CfdpStates::IDLE) {
    // According to standard, discard metadata PDU if we are busy
    return OK;
  }
  ReturnValue_t result = OK;
  fsmRes.step = TransactionStep::TRANSACTION_START;
  if (reader.getTransmissionMode() == TransmissionMode::UNACKNOWLEDGED) {
    fsmRes.state = CfdpStates::BUSY_CLASS_1_NACKED;
  } else if (reader.getTransmissionMode() == TransmissionMode::ACKNOWLEDGED) {
    fsmRes.state = CfdpStates::BUSY_CLASS_2_ACKED;
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
  if (not dp.remoteCfgTable.getRemoteCfg(tp.pduConf.sourceId, &tp.remoteCfg)) {
    // TODO: Warning, event etc.
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "cfdp::DestHandler" << __func__
                 << ": No remote configuration found for destination ID "
                 << tp.pduConf.sourceId.getValue() << std::endl;
#endif
    return FAILED;
  }
  // If both dest name size and source name size are 0, we are dealing with a metadata only PDU,
  // so there is no need to create a file or truncate an existing file
  if (destNameSize > 0 and sourceNameSize > 0) {
    FilesystemParams fparams(tp.destName.data());
    // TODO: Filesystem errors?
    if (dp.user.vfs.fileExists(fparams)) {
      dp.user.vfs.truncateFile(fparams);
    } else {
      result = dp.user.vfs.createFile(fparams);
      if (result != OK) {
        // TODO: Handle FS error. This is probably a case for the filestore rejection mechanism of
        // CFDP.
        //       In any case, it does not really make sense to continue here
      }
    }
  }
  fsmRes.step = TransactionStep::RECEIVING_FILE_DATA_PDUS;
  MetadataRecvdParams params(tp.transactionId, tp.pduConf.sourceId);
  params.fileSize = tp.fileSize.getSize();
  params.destFileName = tp.destName.data();
  params.sourceFileName = tp.sourceName.data();
  params.msgsToUserArray = dynamic_cast<MessageToUserTlv*>(userTlvVec.data());
  params.msgsToUserLen = info.getOptionsLen();
  dp.user.metadataRecvdIndication(params);
  return result;
}

cfdp::CfdpStates cfdp::DestHandler::getCfdpState() const { return fsmRes.state; }

ReturnValue_t cfdp::DestHandler::handleTransferCompletion() {
  ReturnValue_t result;
  if (tp.checksumType != ChecksumType::NULL_CHECKSUM) {
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
  if (fsmRes.state == CfdpStates::BUSY_CLASS_1_NACKED) {
    if (tp.closureRequested) {
      fsmRes.step = TransactionStep::SENDING_FINISHED_PDU;
    } else {
      finish();
    }
  } else if (fsmRes.state == CfdpStates::BUSY_CLASS_2_ACKED) {
    fsmRes.step = TransactionStep::SENDING_FINISHED_PDU;
  }
  return OK;
}

void cfdp::DestHandler::finish() {
  tp.reset();
  dp.packetListRef.clear();
  fsmRes.state = CfdpStates::IDLE;
  fsmRes.step = TransactionStep::IDLE;
}

ReturnValue_t cfdp::DestHandler::checksumVerification() {
  std::array<uint8_t, 1024> buf{};
  // TODO: Checksum verification and notice of completion
  etl::crc32 crcCalc;
  uint64_t currentOffset = 0;
  FileOpParams params(tp.destName.data(), tp.fileSize.value());
  while (currentOffset < tp.fileSize.value()) {
    uint64_t readLen;
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
        // TODO: I think this is a case for a filestore rejection, but it might sense to print
        //       a warning or trigger an event because this should generally not happen
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
    sif::warning << "CRC check for file " << tp.destName.data() << " failed" << std::endl;
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
  result = fp.msgQueue->sendMessage(fp.packetDest.getReportReceptionQueue(), &msg);
  if (result != OK) {
    // TODO: Error handling and event, this is a non CFDP specific error (most likely store is full)
    return result;
  }
  fsmRes.packetsSent++;
  return OK;
}

cfdp::DestHandler::TransactionStep cfdp::DestHandler::getTransactionStep() const {
  return fsmRes.step;
}

const cfdp::DestHandler::FsmResult& cfdp::DestHandler::updateFsmRes(uint8_t errors) {
  fsmRes.errors = errors;
  fsmRes.result = OK;
  if (fsmRes.errors > 0) {
    fsmRes.result = FAILED;
  }
  return fsmRes;
}

const cfdp::TransactionId& cfdp::DestHandler::getTransactionId() const { return tp.transactionId; }

void cfdp::DestHandler::checkAndHandleError(ReturnValue_t result, uint8_t& errorIdx) {
  if (result != OK and errorIdx < 3) {
    fsmRes.errorCodes[errorIdx] = result;
    errorIdx++;
  }
}

void cfdp::DestHandler::setMsgQueue(MessageQueueIF& queue) { fp.msgQueue = &queue; }

void cfdp::DestHandler::setEventReporter(EventReportingProxyIF& reporter) {
  fp.eventReporter = &reporter;
}

const cfdp::DestHandlerParams& cfdp::DestHandler::getDestHandlerParams() const { return dp; }

StorageManagerIF* cfdp::DestHandler::getTmStore() const { return fp.tmStore; }
StorageManagerIF* cfdp::DestHandler::getTcStore() const { return fp.tcStore; }
