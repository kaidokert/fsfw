#ifndef FSFW_CFDP_USERBASE_H
#define FSFW_CFDP_USERBASE_H

#include <optional>
#include <utility>

#include "StatusReportIF.h"
#include "fsfw/cfdp/VarLenFields.h"
#include "fsfw/cfdp/tlv/FilestoreResponseTlv.h"
#include "fsfw/cfdp/tlv/MessageToUserTlv.h"
#include "fsfw/filesystem/HasFileSystemIF.h"

namespace cfdp {

struct TransactionFinishedParams {
  TransactionFinishedParams(TransactionId id, ConditionCode code, FileDeliveryStatus status,
                            FileDeliveryCode delivCode)
      : id(std::move(id)), condCode(code), status(status), deliveryCode(delivCode) {}

  TransactionId id;
  ConditionCode condCode;
  FileDeliveryStatus status;
  FileDeliveryCode deliveryCode;
  std::pair<uint8_t, FilestoreResponseTlv**> fsResponses;
  StatusReportIF* statusReport = nullptr;
};

struct MetadataRecvParams {
  TransactionId id;
  EntityId sourceId;
  size_t fileSize;
  const char* sourceFileName;
  const char* destFileName;
  std::pair<uint8_t, MessageToUserTlv**> msgsToUser;
};

struct FileSegmentRecvdParams {
  TransactionId id;
  size_t offset;
  size_t length;
  std::optional<RecordContinuationState> recContState = std::nullopt;
  std::pair<const uint8_t*, size_t> segmentMetadata;
};

class UserBase {
 public:
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
   * @param vfs Virtual Filestore Object. Will be used for all file operations
   */
  explicit UserBase(HasFileSystemIF& vfs);

  virtual void transactionIndication(TransactionId id) = 0;
  virtual void eofSentIndication(TransactionId id) = 0;

  virtual void abandonedIndication(TransactionId id, ConditionCode code, uint64_t progress) = 0;
  virtual void eofRecvIndication(TransactionId id) = 0;

  // TODO: Parameters
  virtual void transactionFinishedIndication(TransactionFinishedParams params) = 0;
  virtual void metadataRecvdIndication(MetadataRecvParams params) = 0;
  virtual void fileSegmentRecvdIndication(FileSegmentRecvdParams params) = 0;
  virtual void reportIndication() = 0;
  virtual void suspendedIndication() = 0;
  virtual void resumedIndication() = 0;

 private:
  HasFileSystemIF& vfs;
};

}  // namespace cfdp

#endif  // FSFW_CFDP_USERBASE_H
