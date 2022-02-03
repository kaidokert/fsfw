#ifndef FSFW_MONITORING_MONITORINGIF_H_
#define FSFW_MONITORING_MONITORINGIF_H_

#include "../serialize/SerializeIF.h"
#include "MonitoringMessage.h"
#include "monitoringConf.h"

class MonitoringIF : public SerializeIF {
 public:
  static const uint8_t VIOLATION_REPORT_MAX_SIZE = 32;
  static const uint8_t LIMIT_TYPE_NO_TYPE = 0xFF;
  static const uint8_t LIMIT_TYPE_LIMIT_CHECK = 0;
  static const uint8_t LIMIT_TYPE_DELTA_CHECK = 1;
  static const uint8_t LIMIT_TYPE_ABSOLUTE_CHECK = 2;
  static const uint8_t LIMIT_TYPE_OBJECT = 128;

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::FDIR_2;
  static const Event MONITOR_CHANGED_STATE = MAKE_EVENT(1, severity::LOW);
  static const Event VALUE_BELOW_LOW_LIMIT = MAKE_EVENT(2, severity::LOW);
  static const Event VALUE_ABOVE_HIGH_LIMIT = MAKE_EVENT(3, severity::LOW);
  static const Event VALUE_OUT_OF_RANGE = MAKE_EVENT(4, severity::LOW);

  static const uint8_t INTERFACE_ID = CLASS_ID::LIMITS_IF;
  static const ReturnValue_t UNCHECKED = MAKE_RETURN_CODE(1);
  static const ReturnValue_t INVALID = MAKE_RETURN_CODE(2);
  static const ReturnValue_t UNSELECTED = MAKE_RETURN_CODE(3);
  static const ReturnValue_t BELOW_LOW_LIMIT = MAKE_RETURN_CODE(4);
  //	static const ReturnValue_t CHECKING_STATUS_BELOW_LOW_THRESHOLD = MAKE_RETURN_CODE(4);
  //	static const ReturnValue_t CHECKING_STATUS_ABOVE_HIGH_THRESHOLD = MAKE_RETURN_CODE(5);
  static const ReturnValue_t ABOVE_HIGH_LIMIT = MAKE_RETURN_CODE(5);
  static const ReturnValue_t UNEXPECTED_VALUE = MAKE_RETURN_CODE(6);
  static const ReturnValue_t OUT_OF_RANGE = MAKE_RETURN_CODE(7);

  static const ReturnValue_t FIRST_SAMPLE = MAKE_RETURN_CODE(0xA0);
  static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(0xE0);
  static const ReturnValue_t WRONG_TYPE = MAKE_RETURN_CODE(0xE1);
  static const ReturnValue_t WRONG_PID = MAKE_RETURN_CODE(0xE2);
  static const ReturnValue_t WRONG_LIMIT_ID = MAKE_RETURN_CODE(0xE3);
  static const ReturnValue_t MONITOR_NOT_FOUND = MAKE_RETURN_CODE(0xEE);

  static const uint8_t REPORT_NONE = 0;
  static const uint8_t REPORT_EVENTS_ONLY = 1;
  static const uint8_t REPORT_REPORTS_ONLY = 2;
  static const uint8_t REPORT_ALL = 3;

  //	static const ReturnValue_t STILL_IN_LOW_WARNING = MAKE_RETURN_CODE(0x11);
  //	static const ReturnValue_t STILL_IN_LOW_LIMIT = MAKE_RETURN_CODE(0x12);
  //	static const ReturnValue_t STILL_IN_HIGH_WARNING = MAKE_RETURN_CODE(0x13);
  //	static const ReturnValue_t STILL_IN_HIGH_LIMIT = MAKE_RETURN_CODE(0x14);
  //	static const ReturnValue_t VARIABLE_IS_INVALID = MAKE_RETURN_CODE(0xE0);
  //	static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(0xE1);
  //	static const ReturnValue_t INVALID_ID = MAKE_RETURN_CODE(0xE2);
  virtual ReturnValue_t check() = 0;
  virtual ReturnValue_t setLimits(uint8_t type, const uint8_t* data, uint32_t size) = 0;
  virtual ReturnValue_t setChecking(uint8_t strategy) = 0;
  virtual ReturnValue_t setToUnchecked() = 0;
  virtual uint8_t getLimitType() const = 0;
  virtual uint32_t getLimitId() const = 0;
  //	virtual ReturnValue_t setEventReporting(bool active) = 0;
  virtual ~MonitoringIF() {}
};

#endif /* FSFW_MONITORING_MONITORINGIF_H_ */
