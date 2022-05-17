#ifndef FSFW_SRC_FSFW_CFDP_PDU_DEFINITIONS_H_
#define FSFW_SRC_FSFW_CFDP_PDU_DEFINITIONS_H_

#include <fsfw/serialize/SerializeIF.h>

#include <cstddef>
#include <cstdint>

#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

namespace cfdp {

static constexpr uint8_t VERSION_BITS = 0b00100000;

static constexpr uint8_t CFDP_CLASS_ID = CLASS_ID::CFDP;

static constexpr ReturnValue_t INVALID_TLV_TYPE =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 1);
static constexpr ReturnValue_t INVALID_DIRECTIVE_FIELDS =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 2);
static constexpr ReturnValue_t INVALID_PDU_DATAFIELD_LEN =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 3);
static constexpr ReturnValue_t INVALID_ACK_DIRECTIVE_FIELDS =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 4);
//! Can not parse options. This can also occur because there are options
//! available but the user did not pass a valid options array
static constexpr ReturnValue_t METADATA_CANT_PARSE_OPTIONS =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 5);
static constexpr ReturnValue_t NAK_CANT_PARSE_OPTIONS =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 6);
static constexpr ReturnValue_t FINISHED_CANT_PARSE_FS_RESPONSES =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 6);
static constexpr ReturnValue_t FILESTORE_REQUIRES_SECOND_FILE =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 8);
//! Can not parse filestore response because user did not pass a valid instance
//! or remaining size is invalid
static constexpr ReturnValue_t FILESTORE_RESPONSE_CANT_PARSE_FS_MESSAGE =
    HasReturnvaluesIF::makeReturnCode(CFDP_CLASS_ID, 9);

//! Checksum types according to the SANA Checksum Types registry
//! https://sanaregistry.org/r/checksum_identifiers/
enum ChecksumType {
  // Modular legacy checksum
  MODULAR = 0,
  CRC_32_PROXIMITY_1 = 1,
  CRC_32C = 2,
  CRC_32 = 3,
  NULL_CHECKSUM = 15
};

enum PduType : bool { FILE_DIRECTIVE = 0, FILE_DATA = 1 };

enum TransmissionModes : bool { ACKNOWLEDGED = 0, UNACKNOWLEDGED = 1 };

enum SegmentMetadataFlag : bool { NOT_PRESENT = 0, PRESENT = 1 };

enum Direction : bool { TOWARDS_RECEIVER = 0, TOWARDS_SENDER = 1 };

enum SegmentationControl : bool {
  NO_RECORD_BOUNDARIES_PRESERVATION = 0,
  RECORD_BOUNDARIES_PRESERVATION = 1
};

enum WidthInBytes : uint8_t {
  // Only those are supported for now
  ONE_BYTE = 1,
  TWO_BYTES = 2,
  FOUR_BYTES = 4,
};

enum FileDirectives : uint8_t {
  INVALID_DIRECTIVE = 0x0f,
  EOF_DIRECTIVE = 0x04,
  FINISH = 0x05,
  ACK = 0x06,
  METADATA = 0x07,
  NAK = 0x08,
  PROMPT = 0x09,
  KEEP_ALIVE = 0x0c
};

enum ConditionCode : uint8_t {
  NO_CONDITION_FIELD = 0xff,
  NO_ERROR = 0b0000,
  POSITIVE_ACK_LIMIT_REACHED = 0b0001,
  KEEP_ALIVE_LIMIT_REACHED = 0b0010,
  INVALID_TRANSMISSION_MODE = 0b0011,
  FILESTORE_REJECTION = 0b0100,
  FILE_CHECKSUM_FAILURE = 0b0101,
  FILE_SIZE_ERROR = 0b0110,
  NAK_LIMIT_REACHED = 0b0111,
  INACTIVITY_DETECTED = 0b1000,
  CHECK_LIMIT_REACHED = 0b1010,
  UNSUPPORTED_CHECKSUM_TYPE = 0b1011,
  SUSPEND_REQUEST_RECEIVED = 0b1110,
  CANCEL_REQUEST_RECEIVED = 0b1111
};

enum AckTransactionStatus {
  UNDEFINED = 0b00,
  ACTIVE = 0b01,
  TERMINATED = 0b10,
  UNRECOGNIZED = 0b11
};

enum FinishedDeliveryCode { DATA_COMPLETE = 0, DATA_INCOMPLETE = 1 };

enum FinishedFileStatus {
  DISCARDED_DELIBERATELY = 0,
  DISCARDED_FILESTORE_REJECTION = 1,
  RETAINED_IN_FILESTORE = 2,
  FILE_STATUS_UNREPORTED = 3
};

enum PromptResponseRequired : bool { PROMPT_NAK = 0, PROMPT_KEEP_ALIVE = 1 };

enum TlvTypes : uint8_t {
  FILESTORE_REQUEST = 0x00,
  FILESTORE_RESPONSE = 0x01,
  MSG_TO_USER = 0x02,
  FAULT_HANDLER = 0x04,
  FLOW_LABEL = 0x05,
  ENTITY_ID = 0x06,
  INVALID_TLV = 0xff,
};

enum RecordContinuationState {
  NO_START_NO_END = 0b00,
  CONTAINS_START_NO_END = 0b01,
  CONTAINS_END_NO_START = 0b10,
  CONTAINS_START_AND_END = 0b11
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_PDU_DEFINITIONS_H_ */
