#ifndef FSFW_CFDP_USERBASE_H
#define FSFW_CFDP_USERBASE_H

#include <optional>
#include <utility>
#include <vector>

#include "StatusReportIF.h"
#include "fsfw/cfdp/VarLenFields.h"
#include "fsfw/cfdp/tlv/FilestoreResponseTlv.h"
#include "fsfw/cfdp/tlv/MessageToUserTlv.h"
#include "fsfw/filesystem/HasFileSystemIF.h"

namespace cfdp {

struct TransactionFinishedParams {
  TransactionFinishedParams(const TransactionId& id, ConditionCode code, FileDeliveryCode delivCode,
                            FileDeliveryStatus status)
      : id(id), condCode(code), status(status), deliveryCode(delivCode) {}

  const TransactionId& id;
  ConditionCode condCode;
  FileDeliveryStatus status;
  FileDeliveryCode deliveryCode;
  std::vector<FilestoreResponseTlv*> fsResponses;
  StatusReportIF* statusReport = nullptr;
};

struct MetadataRecvdParams {
  MetadataRecvdParams(const TransactionId& id, const EntityId& sourceId)
      : id(id), sourceId(sourceId) {}
  const TransactionId& id;
  const EntityId& sourceId;
  uint64_t fileSize = 0;
  const char* sourceFileName = "";
  const char* destFileName = "";
  size_t msgsToUserLen = 0;
  const MessageToUserTlv* msgsToUserArray = nullptr;
};

struct FileSegmentRecvdParams {
  TransactionId id;
  size_t offset;
  size_t length;
  std::optional<RecordContinuationState> recContState = std::nullopt;
  std::pair<const uint8_t*, size_t> segmentMetadata;
};

/**
 * @brief Base class which provides a user interface to interact with CFDP handlers.
 *
 * @details
 * This class is also used to pass the Virtual Filestore (VFS) Implementation to the CFDP
 * handlers so the filestore operations can be mapped to the underlying filestore.
 *
 * It is used by implementing it in a child class and then passing it to the CFDP
 * handler objects. The base class provides default implementation for the user indication
 * primitives specified in the CFDP standard. The user can override these implementations
 * to provide custom indication handlers.
 *
 * Please note that for all indication callbacks, the passed transaction ID reference will
 * become invalid shortly after the function has been executed. If the transaction ID is to be
 * cached or used, create an own copy of it.
 * @param vfs Virtual Filestore Object. Will be used for all file operations
 */
class UserBase {
  friend class DestHandler;

 public:
  explicit UserBase(HasFileSystemIF& vfs);

  virtual void transactionIndication(const TransactionId& id) = 0;
  virtual void eofSentIndication(const TransactionId& id) = 0;
  virtual void transactionFinishedIndication(const TransactionFinishedParams& params) = 0;
  /**
   * Will be called if metadata was received.
   *
   * IMPORTANT: The passed struct contains the messages to the user in form of a raw C array.
   * The TLVs in these arrays are zero-copy types, which means that they point to the raw data
   * inside the metadata packet directly. The metadata packet will be deleted from the TC store
   * shortly after it was processed. If some of the data is to be cached and/or used after the
   * function call, it needs to be copied into another user-provided buffer.
   * @param params
   */
  virtual void metadataRecvdIndication(const MetadataRecvdParams& params) = 0;
  virtual void fileSegmentRecvdIndication(const FileSegmentRecvdParams& params) = 0;
  virtual void reportIndication(const TransactionId& id, StatusReportIF& report) = 0;
  virtual void suspendedIndication(const TransactionId& id, ConditionCode code) = 0;
  virtual void resumedIndication(const TransactionId& id, size_t progress) = 0;
  virtual void faultIndication(const TransactionId& id, ConditionCode code, size_t progress) = 0;
  virtual void abandonedIndication(const TransactionId& id, ConditionCode code,
                                   size_t progress) = 0;
  virtual void eofRecvIndication(const TransactionId& id) = 0;

 private:
  HasFileSystemIF& vfs;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_USERBASE_H
