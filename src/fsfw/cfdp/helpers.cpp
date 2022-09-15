#include "helpers.h"

const char* COND_CODE_STRINGS[14] = {"Unknown",
                                     "No Error",
                                     "Positive ACK Limit Reached",
                                     "Keep Alive Limit Reached",
                                     "Invalid Transmission Mode",
                                     "Filestore Rejection",
                                     "File Checksum Failure",
                                     "File Size Error",
                                     "NAK limit reached",
                                     "Inactivity Detected",
                                     "Check Limit Reached",
                                     "Unsupported Checksum Type",
                                     "Suspend Request Received",
                                     "Cancel Request Received"};

const char* cfdp::getConditionCodeString(cfdp::ConditionCode code) {
  switch (code) {
    case NO_CONDITION_FIELD:
      return COND_CODE_STRINGS[0];
    case NO_ERROR:
      return COND_CODE_STRINGS[1];
    case POSITIVE_ACK_LIMIT_REACHED:
      return COND_CODE_STRINGS[2];
    case KEEP_ALIVE_LIMIT_REACHED:
      return COND_CODE_STRINGS[3];
    case INVALID_TRANSMISSION_MODE:
      return COND_CODE_STRINGS[4];
    case FILESTORE_REJECTION:
      return COND_CODE_STRINGS[5];
    case FILE_CHECKSUM_FAILURE:
      return COND_CODE_STRINGS[6];
    case FILE_SIZE_ERROR:
      return COND_CODE_STRINGS[7];
    case NAK_LIMIT_REACHED:
      return COND_CODE_STRINGS[8];
    case INACTIVITY_DETECTED:
      return COND_CODE_STRINGS[9];
    case CHECK_LIMIT_REACHED:
      return COND_CODE_STRINGS[10];
    case UNSUPPORTED_CHECKSUM_TYPE:
      return COND_CODE_STRINGS[11];
    case SUSPEND_REQUEST_RECEIVED:
      return COND_CODE_STRINGS[12];
    case CANCEL_REQUEST_RECEIVED:
      return COND_CODE_STRINGS[13];
  }
  return "Unknown";
}
