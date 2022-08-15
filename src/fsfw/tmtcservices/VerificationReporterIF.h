#ifndef FSFW_TMTCSERVICES_VERIFICATIONREPORTERIF_H
#define FSFW_TMTCSERVICES_VERIFICATIONREPORTERIF_H

#include "fsfw/retval.h"
#include "fsfw/tmtcpacket/pus/tc.h"

struct VerifParamsBase {
  VerifParamsBase() : reportId(0), tcPacketId(0), tcPsc(0) {}
  VerifParamsBase(uint8_t reportId, uint16_t tcPacketId, uint16_t tcPsc)
      : reportId(reportId), tcPacketId(tcPacketId), tcPsc(tcPsc) {}
  uint8_t reportId;
  uint16_t tcPacketId;
  uint16_t tcPsc;
  uint8_t ackFlags = ecss::ACK_ALL;
  uint8_t step = 0;

  void resetTcFields() {
    tcPacketId = 0;
    tcPsc = 0;
    ackFlags = 0;
  }
};

struct VerifSuccessParams : public VerifParamsBase {
  VerifSuccessParams() = default;
  VerifSuccessParams(uint8_t reportId, uint16_t tcPacketId, uint16_t tcPsc)
      : VerifParamsBase(reportId, tcPacketId, tcPsc) {}
  VerifSuccessParams(uint8_t reportId, PusTcIF& tc)
      : VerifParamsBase(reportId, tc.getPacketIdRaw(), tc.getPacketSeqCtrlRaw()) {}
};

struct VerifFailureParams : public VerifParamsBase {
  VerifFailureParams() = default;
  VerifFailureParams(uint8_t reportId, uint16_t tcPacketId, uint16_t tcPsc, ReturnValue_t errorCode,
                     uint32_t errorParam1, uint32_t errorParams2)
      : VerifParamsBase(reportId, tcPacketId, tcPsc),
        errorCode(errorCode),
        errorParam1(errorParam1),
        errorParam2(errorParams2) {}
  VerifFailureParams(uint8_t reportId, uint16_t tcPacketId, uint16_t tcPsc, ReturnValue_t errorCode)
      : VerifParamsBase(reportId, tcPacketId, tcPsc), errorCode(errorCode) {}
  VerifFailureParams(uint8_t reportId, uint16_t tcPacketId, uint16_t tcPsc)
      : VerifParamsBase(reportId, tcPacketId, tcPsc) {}
  VerifFailureParams(uint8_t reportId, PusTcIF& tc, ReturnValue_t errorCode)
      : VerifParamsBase(reportId, tc.getPacketIdRaw(), tc.getPacketSeqCtrlRaw()),
        errorCode(errorCode) {}
  VerifFailureParams(uint8_t reportId, PusTcIF& tc)
      : VerifParamsBase(reportId, tc.getPacketIdRaw(), tc.getPacketSeqCtrlRaw()) {}

  void resetFailParams() {
    errorCode = returnvalue::FAILED;
    errorParam1 = 0;
    errorParam2 = 0;
  }

  ReturnValue_t errorCode = returnvalue::FAILED;
  uint8_t step = 0;
  uint32_t errorParam1 = 0;
  uint32_t errorParam2 = 0;
};
class VerificationReporterIF {
 public:
  virtual ~VerificationReporterIF() = default;

  virtual ReturnValue_t sendSuccessReport(VerifSuccessParams params) = 0;
  virtual ReturnValue_t sendFailureReport(VerifFailureParams params) = 0;
};

#endif  // FSFW_TMTCSERVICES_VERIFICATIONREPORTERIF_H
