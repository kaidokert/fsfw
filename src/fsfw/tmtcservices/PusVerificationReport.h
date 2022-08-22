#ifndef FSFW_TMTCSERVICES_PUSVERIFICATIONREPORT_H_
#define FSFW_TMTCSERVICES_PUSVERIFICATIONREPORT_H_

#include "VerificationCodes.h"
#include "fsfw/ipc/MessageQueueMessage.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tmtcpacket/pus/tc/PusTcReader.h"

class PusVerificationMessage : public MessageQueueMessage {
 private:
  struct verifciationMessageContent {
    uint8_t reportId;
    uint8_t ackFlags;
    uint8_t packetId_0;
    uint8_t packetId_1;
    uint8_t tcSequenceControl_0;
    uint8_t tcSequenceControl_1;
    uint8_t step;
    uint8_t error_code_0;
    uint8_t error_code_1;
    uint8_t parameter1_0;
    uint8_t parameter1_1;
    uint8_t parameter1_2;
    uint8_t parameter1_3;
    uint8_t parameter2_0;
    uint8_t parameter2_1;
    uint8_t parameter2_2;
    uint8_t parameter2_3;
  };
  verifciationMessageContent* getContent();

 public:
  static const uint8_t VERIFICATION_MIN_SIZE = 6;
  PusVerificationMessage();

  PusVerificationMessage(uint8_t set_report_id, uint8_t ackFlags, uint16_t tcPacketId,
                         uint16_t tcSequenceControl, ReturnValue_t set_error_code = 0,
                         uint8_t set_step = 0, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
  uint8_t getReportId();
  uint8_t getAckFlags();
  uint16_t getTcPacketId();
  uint16_t getTcSequenceControl();
  ReturnValue_t getErrorCode();
  uint8_t getStep();
  uint32_t getParameter1();
  uint32_t getParameter2();
};

class PusSuccessReport {
 private:
  static const uint16_t MAX_SIZE = 7;
  uint8_t reportBuffer[MAX_SIZE];
  size_t reportSize;
  uint8_t* pBuffer;

 public:
  PusSuccessReport(uint16_t setPacketId, uint16_t setSequenceControl, uint8_t set_step = 0);
  ~PusSuccessReport();
  uint32_t getSize();
  uint8_t* getReport();
};

class PusFailureReport {
 private:
  static const uint16_t MAX_SIZE = 16;
  uint8_t reportBuffer[MAX_SIZE];
  size_t reportSize;
  uint8_t* pBuffer;

 public:
  PusFailureReport(uint16_t setPacketId, uint16_t setSequenceControl, ReturnValue_t setErrorCode,
                   uint8_t setStep = 0, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
  ~PusFailureReport();
  size_t getSize();
  uint8_t* getReport();
};

#endif /* FSFW_TMTCSERVICES_PUSVERIFICATIONREPORT_H_ */
