#ifndef FRAMEWORK_MONITORING_TRIPLEXMONITOR_H_
#define FRAMEWORK_MONITORING_TRIPLEXMONITOR_H_

#include "../datapool/DataSet.h"
#include "../datapool/PIDReaderList.h"
#include "../health/HealthTableIF.h"
#include "../objectmanager/ObjectManager.h"
#include "../parameters/HasParametersIF.h"
#include "monitoringConf.h"

// SHOULDDO: This is by far not perfect. Could be merged with new Monitor classes. But still, it's
// over-engineering.
template <typename T>
class TriplexMonitor : public HasParametersIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::TRIPLE_REDUNDACY_CHECK;
  static const ReturnValue_t NOT_ENOUGH_SENSORS = MAKE_RETURN_CODE(1);
  static const ReturnValue_t LOWEST_VALUE_OOL = MAKE_RETURN_CODE(2);
  static const ReturnValue_t HIGHEST_VALUE_OOL = MAKE_RETURN_CODE(3);
  static const ReturnValue_t BOTH_VALUES_OOL = MAKE_RETURN_CODE(4);
  static const ReturnValue_t DUPLEX_OOL = MAKE_RETURN_CODE(5);

  static const uint8_t THREE = 3;

  TriplexMonitor(const uint32_t parameterIds[3], uint8_t domainId, const T initialLimit,
                 Event eventTripleCheck, Event eventDualCheck)
      : values(parameterIds, &dataSet),
        limit(initialLimit),
        eventTripleCheck(eventTripleCheck),
        eventDualCheck(eventDualCheck),
        healthTable(NULL),
        domainId(domainId) {}
  virtual ~TriplexMonitor() {}
  ReturnValue_t check() {
    dataSet.read();
    // check health and validity
    uint8_t availableIndex[2] = {0, 0};
    bool first = true;
    uint8_t nAvailable = 0;
    for (uint8_t count = 0; count < THREE; count++) {
      if (values[count].isValid() && checkObjectHealthState(count)) {
        if (first) {
          availableIndex[0] = count;
          first = false;
        } else {
          // Might be filled twice, but then it's not needed anyway.
          availableIndex[1] = count;
        }
        nAvailable++;
      }
    }
    ReturnValue_t result = returnvalue::FAILED;
    switch (nAvailable) {
      case 3:
        result = doTriplexMonitoring();
        break;
      case 2:
        result = doDuplexMonitoring(availableIndex);
        break;
      default:
        result = NOT_ENOUGH_SENSORS;
        break;
    }
    dataSet.commit();
    return result;
  }
  ReturnValue_t initialize() {
    healthTable = ObjectManager::instance()->get<HealthTableIF>(objects::HEALTH_TABLE);
    if (healthTable == NULL) {
      return returnvalue::FAILED;
    }
    return returnvalue::OK;
  }

  ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId, ParameterWrapper *parameterWrapper,
                             const ParameterWrapper *newValues, uint16_t startAtIndex) {
    if (domainId != this->domainId) {
      return INVALID_DOMAIN_ID;
    }
    switch (uniqueId) {
      case 0:
        parameterWrapper->set(limit);
        break;
      default:
        return INVALID_IDENTIFIER_ID;
    }
    return returnvalue::OK;
  }

 protected:
  DataSet dataSet;
  PIDReaderList<T, THREE> values;
  T limit;
  Event eventTripleCheck;
  Event eventDualCheck;
  HealthTableIF *healthTable;
  uint8_t domainId;
  ReturnValue_t doTriplexMonitoring() {
    ReturnValue_t result = returnvalue::OK;
    // Find middle value, by ordering indices
    uint8_t index[3] = {0, 1, 2};
    if (values[index[0]].value > values[index[1]].value) {
      std::swap(index[0], index[1]);
    }
    if (values[index[0]].value > values[index[2]].value) {
      std::swap(index[0], index[2]);
    }
    if (values[index[1]].value > values[index[2]].value) {
      std::swap(index[1], index[2]);
    }
    // Test if smallest value is out-of-limit.
    if (values[index[0]] < (values[index[1]] - limit)) {
      EventManagerIF::triggerEvent(getRefereneceObject(index[0]), eventTripleCheck,
                                   LOWEST_VALUE_OOL, 0);
      result = LOWEST_VALUE_OOL;
    }
    // Test if largest value is out-of-limit.
    if (values[index[2]] > (values[index[1]] + limit)) {
      EventManagerIF::triggerEvent(getRefereneceObject(index[2]), eventTripleCheck,
                                   HIGHEST_VALUE_OOL, 0);
      if (result == returnvalue::OK) {
        result = HIGHEST_VALUE_OOL;
      } else {
        result = BOTH_VALUES_OOL;
      }
    }
    return result;
  }

  ReturnValue_t doDuplexMonitoring(uint8_t index[2]) {
    T mean = (values[index[0]] + values[index[1]]) / 2;
    if (values[index[0]] > values[index[1]]) {
      if (values[index[0]] > (mean + limit)) {
        EventManagerIF::triggerEvent(getRefereneceObject(index[0]), eventDualCheck, 0, 0);
        EventManagerIF::triggerEvent(getRefereneceObject(index[1]), eventDualCheck, 0, 0);
        return DUPLEX_OOL;
      }
    } else {
      if (values[index[1]] > (mean + limit)) {
        EventManagerIF::triggerEvent(getRefereneceObject(index[0]), eventDualCheck, 0, 0);
        EventManagerIF::triggerEvent(getRefereneceObject(index[1]), eventDualCheck, 0, 0);
        return DUPLEX_OOL;
      }
    }
    return returnvalue::OK;
  }
  virtual bool checkObjectHealthState(uint8_t valueIndex) = 0;
  virtual object_id_t getRefereneceObject(uint8_t valueIndex) = 0;
};

#endif /* FRAMEWORK_MONITORING_TRIPLEXMONITOR_H_ */
