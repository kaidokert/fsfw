#ifndef MONITORINGMESSAGECONTENT_H_
#define MONITORINGMESSAGECONTENT_H_

#include <framework/monitoring/HasMonitorsIF.h>
#include <framework/monitoring/MonitoringIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serialize/SerialFixedArrayListAdapter.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/timemanager/TimeStamperIF.h>

//PID(uint32_t), TYPE, LIMIT_ID, value,limitValue, previous, later, timestamp
template<typename T>
class MonitoringReportContent: public SerialLinkedListAdapter<SerializeIF> {
public:
	SerializeElement<uint8_t> monitorId;
	SerializeElement<uint32_t> parameterId;
	SerializeElement<T> parameterValue;
	SerializeElement<T> limitValue;
	SerializeElement<ReturnValue_t> oldState;
	SerializeElement<ReturnValue_t> newState;
	uint8_t rawTimestamp[TimeStamperIF::MISSION_TIMESTAMP_SIZE];
	SerializeElement<SerialBufferAdapter> timestampSerializer;
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
					LinkedElement<SerializeIF>::Iterator(&parameterId)), parameterId(
					setPID), parameterValue(value), limitValue(limitValue), oldState(
					oldState), newState(newState), rawTimestamp( { 0 }), timestampSerializer(rawTimestamp,
							sizeof(rawTimestamp)), timeStamper(NULL) {
		setAllNext();
		if (checkAndSetStamper()) {
			timeStamper->addTimeStamp(rawTimestamp, sizeof(rawTimestamp));
		}
	}
private:
	void setAllNext() {
		parameterId.setNext(&parameterValue);
		parameterValue.setNext(&limitValue);
		limitValue.setNext(&oldState);
		oldState.setNext(&newState);
		newState.setNext(&timestampSerializer);
	}
	bool checkAndSetStamper() {
		if (timeStamper == NULL) {
			//TODO: Adjust name?
			timeStamper = objectManager->get<TimeStamperIF>( objects::TIME_MANAGER );
			if ( timeStamper == NULL ) {
				error << "MonitoringReportContent::checkAndSetStamper: Stamper not found!" << std::endl;
				return false;
			}
		}
		return true;
	}
};

//TODO: Next message would be update_limit message.
//PID(uint32_t), Data{type, n_entries {LIMIT_ID(uint8_t), TYPE(uint8_t) newLimits(data, depends)

template<typename T>
class UpdateLimitMonitorContent: public SerialLinkedListAdapter<SerializeIF> {
public:
	SerializeElement<T> lowValue;
	SerializeElement<T> highValue;
	UpdateLimitMonitorContent() :
			SerialLinkedListAdapter<SerializeIF>(&lowValue), lowValue(0), highValue(
					0) {
		lowValue.setNext(&highValue);
	}
private:
};

//Not used at the moment.
//class EnableDisableInner: public SerialLinkedListAdapter<SerializeIF> {
//public:
//	SerializeElement<uint32_t> parameterId;
//	typedef FixedArrayList<uint32_t, HasMonitorsIF::MAX_N_LIMIT_ID> LimitIdList;
//	SerializeElement<LimitIdList> limitList;
//	EnableDisableInner() :
//			SerialLinkedListAdapter<SerializeIF>(&parameterId) {
//		parameterId.setNext(&limitList);
//	}
//};

class EnableDisableContent {
public:
	typedef SerialFixedArrayListAdapter<uint32_t, HasMonitorsIF::MAX_N_PARAMETER> EnableDisableList;
	EnableDisableList funkyList;
	EnableDisableContent() {
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
	bool bigEndian) {
		return funkyList.deSerialize(buffer, size, bigEndian);
	}
};

#endif /* MONITORINGMESSAGECONTENT_H_ */
