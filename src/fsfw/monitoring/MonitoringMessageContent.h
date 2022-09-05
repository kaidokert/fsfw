#ifndef FSFW_MONITORING_MONITORINGMESSAGECONTENT_H_
#define FSFW_MONITORING_MONITORINGMESSAGECONTENT_H_

#include "../datapoollocal/localPoolDefinitions.h"
#include "../objectmanager/ObjectManager.h"
#include "../serialize/SerialBufferAdapter.h"
#include "../serialize/SerialFixedArrayListAdapter.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../serialize/SerializeElement.h"
#include "../serviceinterface/ServiceInterface.h"
#include "../timemanager/TimeWriterIF.h"
#include "HasMonitorsIF.h"
#include "MonitoringIF.h"
#include "monitoringConf.h"

namespace Factory {
void setStaticFrameworkObjectIds();
}

// PID(uint32_t), TYPE, LIMIT_ID, value,limitValue, previous, later, timestamp
/**
 * @brief	Does magic.
 * @tparam T
 */
template <typename T>
class MonitoringReportContent : public SerialLinkedListAdapter<SerializeIF> {
  friend void(Factory::setStaticFrameworkObjectIds)();

 public:
  SerializeElement<uint8_t> monitorId;
  SerializeElement<uint32_t> parameterObjectId;
  SerializeElement<lp_id_t> localPoolId;
  SerializeElement<T> parameterValue;
  SerializeElement<T> limitValue;
  SerializeElement<ReturnValue_t> oldState;
  SerializeElement<ReturnValue_t> newState;
  uint8_t rawTimestamp[TimeWriterIF::MAXIMUM_TIMESTAMP_LEN] = {};
  SerializeElement<SerialBufferAdapter<uint8_t>> timestampSerializer;
  TimeWriterIF* timeStamper;
  MonitoringReportContent()
      : SerialLinkedListAdapter<SerializeIF>(&parameterObjectId),
        monitorId(0),
        parameterObjectId(0),
        localPoolId(0),
        parameterValue(0),
        limitValue(0),
        oldState(0),
        newState(0),
        timestampSerializer(rawTimestamp, sizeof(rawTimestamp)),
        timeStamper(nullptr) {
    setAllNext();
  }
  MonitoringReportContent(gp_id_t globalPoolId, T value, T limitValue, ReturnValue_t oldState,
                          ReturnValue_t newState)
      : SerialLinkedListAdapter<SerializeIF>(&parameterObjectId),
        monitorId(0),
        parameterObjectId(globalPoolId.objectId),
        localPoolId(globalPoolId.localPoolId),
        parameterValue(value),
        limitValue(limitValue),
        oldState(oldState),
        newState(newState),
        timestampSerializer(rawTimestamp, sizeof(rawTimestamp)),
        timeStamper(nullptr) {
    setAllNext();
    if (checkAndSetStamper()) {
      timeStamper->addTimeStamp(rawTimestamp, sizeof(rawTimestamp));
    }
  }

 private:
  static object_id_t timeStamperId;
  void setAllNext() {
    parameterObjectId.setNext(&parameterValue);
    parameterValue.setNext(&limitValue);
    limitValue.setNext(&oldState);
    oldState.setNext(&newState);
    newState.setNext(&timestampSerializer);
  }
  bool checkAndSetStamper() {
    if (timeStamper == nullptr) {
      timeStamper = ObjectManager::instance()->get<TimeWriterIF>(timeStamperId);
      if (timeStamper == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "MonitoringReportContent::checkAndSetStamper: "
                      "Stamper not found!"
                   << std::endl;
#endif
        return false;
      }
    }
    return true;
  }
};
template <typename T>
object_id_t MonitoringReportContent<T>::timeStamperId = 0;

#endif /* FSFW_MONITORING_MONITORINGMESSAGECONTENT_H_ */
