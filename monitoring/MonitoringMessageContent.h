#ifndef MONITORINGMESSAGECONTENT_H_
#define MONITORINGMESSAGECONTENT_H_

#include "HasMonitorsIF.h"
#include "MonitoringIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../serialize/SerialBufferAdapter.h"
#include "../serialize/SerialFixedArrayListAdapter.h"
#include "../serialize/SerializeElement.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../timemanager/TimeStamperIF.h"

namespace Factory{
void setStaticFrameworkObjectIds();
}

//PID(uint32_t), TYPE, LIMIT_ID, value,limitValue, previous, later, timestamp
template<typename T>
class MonitoringReportContent: public SerialLinkedListAdapter<SerializeIF> {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	SerializeElement<uint8_t> monitorId;
	SerializeElement<uint32_t> parameterId;
	SerializeElement<T> parameterValue;
	SerializeElement<T> limitValue;
	SerializeElement<ReturnValue_t> oldState;
	SerializeElement<ReturnValue_t> newState;
	uint8_t rawTimestamp[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
	SerializeElement<SerialBufferAdapter<uint8_t>> timestampSerializer;
	TimeStamperIF* timeStamper;
	MonitoringReportContent() :
			SerialLinkedListAdapter<SerializeIF>(
					LinkedElement<SerializeIF>::Iterator(&parameterId)), monitorId(0), parameterId(
					0), parameterValue(0), limitValue(0), oldState(0), newState(
					0), rawTimestamp( { 0 }), timestampSerializer(rawTimestamp,
					sizeof(rawTimestamp)), timeStamper(NULL) {
		setAllNext();
	}
	MonitoringReportContent(uint32_t setPID, T value, T limitValue,
			ReturnValue_t oldState, ReturnValue_t newState) :
			SerialLinkedListAdapter<SerializeIF>(
					LinkedElement<SerializeIF>::Iterator(&parameterId)), monitorId(0), parameterId(
					setPID), parameterValue(value), limitValue(limitValue), oldState(
					oldState), newState(newState), timestampSerializer(rawTimestamp,
							sizeof(rawTimestamp)), timeStamper(NULL) {
		setAllNext();
		if (checkAndSetStamper()) {
			timeStamper->addTimeStamp(rawTimestamp, sizeof(rawTimestamp));
		}
	}
private:

	static object_id_t timeStamperId;
	void setAllNext() {
		parameterId.setNext(&parameterValue);
		parameterValue.setNext(&limitValue);
		limitValue.setNext(&oldState);
		oldState.setNext(&newState);
		newState.setNext(&timestampSerializer);
	}
	bool checkAndSetStamper() {
		if (timeStamper == NULL) {
			timeStamper = objectManager->get<TimeStamperIF>( timeStamperId );
			if ( timeStamper == NULL ) {
				sif::error << "MonitoringReportContent::checkAndSetStamper: "
				        "Stamper not found!" << std::endl;
				return false;
			}
		}
		return true;
	}
};
template<typename T>
object_id_t MonitoringReportContent<T>::timeStamperId = 0;

#endif /* MONITORINGMESSAGECONTENT_H_ */
