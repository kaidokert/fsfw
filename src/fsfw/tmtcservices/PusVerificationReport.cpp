#include "fsfw/tmtcservices/PusVerificationReport.h"

#include "fsfw/serialize/SerializeAdapter.h"

PusVerificationMessage::PusVerificationMessage() {}

PusVerificationMessage::PusVerificationMessage(uint8_t set_report_id, uint8_t ackFlags,
                                               uint16_t tcPacketId, uint16_t tcSequenceControl,
                                               ReturnValue_t set_error_code, uint8_t set_step,
                                               uint32_t parameter1, uint32_t parameter2) {
  uint8_t* data = this->getBuffer();
  data[messageSize] = set_report_id;
  messageSize += sizeof(set_report_id);
  data[messageSize] = ackFlags;
  messageSize += sizeof(ackFlags);
  memcpy(&data[messageSize], &tcPacketId, sizeof(tcPacketId));
  messageSize += sizeof(tcPacketId);
  memcpy(&data[messageSize], &tcSequenceControl, sizeof(tcSequenceControl));
  messageSize += sizeof(tcSequenceControl);
  data[messageSize] = set_step;
  messageSize += sizeof(set_step);
  memcpy(&data[messageSize], &set_error_code, sizeof(set_error_code));
  messageSize += sizeof(set_error_code);
  memcpy(&data[messageSize], &parameter1, sizeof(parameter1));
  messageSize += sizeof(parameter1);
  memcpy(&data[messageSize], &parameter2, sizeof(parameter2));
  messageSize += sizeof(parameter2);
}

uint8_t PusVerificationMessage::getReportId() { return getContent()->reportId; }

uint8_t PusVerificationMessage::getAckFlags() { return getContent()->ackFlags; }

uint16_t PusVerificationMessage::getTcPacketId() {
  uint16_t tcPacketId;
  memcpy(&tcPacketId, &getContent()->packetId_0, sizeof(tcPacketId));
  return tcPacketId;
}

uint16_t PusVerificationMessage::getTcSequenceControl() {
  uint16_t tcSequenceControl;
  memcpy(&tcSequenceControl, &getContent()->tcSequenceControl_0, sizeof(tcSequenceControl));
  return tcSequenceControl;
}

uint8_t PusVerificationMessage::getStep() { return getContent()->step; }

ReturnValue_t PusVerificationMessage::getErrorCode() {
  ReturnValue_t errorCode;
  memcpy(&errorCode, &getContent()->error_code_0, sizeof(errorCode));
  return errorCode;
}

PusVerificationMessage::verifciationMessageContent* PusVerificationMessage::getContent() {
  return (verifciationMessageContent*)this->getData();
}

uint32_t PusVerificationMessage::getParameter1() {
  uint32_t parameter;
  memcpy(&parameter, &getContent()->parameter1_0, sizeof(parameter));
  return parameter;
}

uint32_t PusVerificationMessage::getParameter2() {
  uint32_t parameter;
  memcpy(&parameter, &getContent()->parameter2_0, sizeof(parameter));
  return parameter;
}

PusSuccessReport::PusSuccessReport(uint16_t setPacketId, uint16_t setSequenceControl,
                                   uint8_t setStep)
    : reportSize(0), pBuffer(reportBuffer) {
  // Serialization won't fail, because we know the necessary max-size of the buffer.
  SerializeAdapter::serialize(&setPacketId, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  SerializeAdapter::serialize(&setSequenceControl, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  if (setStep != 0) {
    SerializeAdapter::serialize(&setStep, &pBuffer, &reportSize, sizeof(reportBuffer),
                                SerializeIF::Endianness::BIG);
  }
}

PusSuccessReport::~PusSuccessReport() {}

uint32_t PusSuccessReport::getSize() { return reportSize; }

uint8_t* PusSuccessReport::getReport() { return reportBuffer; }

PusFailureReport::PusFailureReport(uint16_t setPacketId, uint16_t setSequenceControl,
                                   ReturnValue_t setErrorCode, uint8_t setStep, uint32_t parameter1,
                                   uint32_t parameter2)
    : reportSize(0), pBuffer(reportBuffer) {
  // Serialization won't fail, because we know the necessary max-size of the buffer.
  SerializeAdapter::serialize(&setPacketId, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  SerializeAdapter::serialize(&setSequenceControl, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  if (setStep != 0) {
    SerializeAdapter::serialize(&setStep, &pBuffer, &reportSize, sizeof(reportBuffer),
                                SerializeIF::Endianness::BIG);
  }
  SerializeAdapter::serialize(&setErrorCode, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  SerializeAdapter::serialize(&parameter1, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
  SerializeAdapter::serialize(&parameter2, &pBuffer, &reportSize, sizeof(reportBuffer),
                              SerializeIF::Endianness::BIG);
}

PusFailureReport::~PusFailureReport() {}

size_t PusFailureReport::getSize() { return reportSize; }

uint8_t* PusFailureReport::getReport() { return reportBuffer; }
