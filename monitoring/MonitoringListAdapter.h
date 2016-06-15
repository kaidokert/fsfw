/*
 * MonitoringListAdapter.h
 *
 *  Created on: 09.07.2014
 *      Author: baetz
 */

#ifndef MONITORINGLISTADAPTER_H_
#define MONITORINGLISTADAPTER_H_

#include <framework/monitoring/MonitoringIF.h>
#include <framework/serialize/SerialLinkedListAdapter.h>

class MonitoringListAdapter : public SerialLinkedListAdapter<MonitoringIF> {
public:
	MonitoringListAdapter(LinkedElement<MonitoringIF>* start) : SerialLinkedListAdapter<MonitoringIF>(start) {
	}
	MonitoringListAdapter() : SerialLinkedListAdapter<MonitoringIF>() {
	}
	/**
	 * Checks all elements.
	 * @return Returns RETURN_FAILED if at least one check failed.
	 */
	ReturnValue_t check();
	/**
	 * Iterates the list to update the requested monitor.
	 * @param parameterId The parameter id to monitor.
	 * @param limitId The limit id for the given PID.
	 * @param data contains the new values
	 * @param size size of the new values
	 * @return The return code of the monitor if it was found and an update attempt, MONITOR_NOT_FOUND else.
	 */
	ReturnValue_t updateMonitor( uint32_t parameterId, uint8_t limitType, const uint8_t* data, uint32_t size);
	/**
	 * Iterates the list to set the checking strategies for one monitor.
	 * @param strategy the reporting strategy.
	 * @param parameterId The PID.
	 * @param limitId The limit id for the given PID.
	 * @return The return code of the monitor if it was found and an update attempt, MONITOR_NOT_FOUND else.
	 */
//	ReturnValue_t setChecking(uint8_t strategy, uint32_t parameterId, uint32_t limitId);
	/**
	 * Iterates the list to set the checking strategies for all monitors of a given parameter.
	 * @param strategy the reporting strategy.
	 * @param parameterId The PID.
	 * @return RETURN_OK if at least one monitor was updated, MONITOR_NOT_FOUND else.
	 */
	ReturnValue_t setChecking(uint8_t strategy, uint32_t parameterId);
	/**
	 * Iterates the list to set the checking strategies for all monitors-
	 * @param strategy the reporting strategy.
	 * @return RETURN_OK if at least one monitor was updated, MONITOR_NOT_FOUND else.
	 */
	ReturnValue_t setChecking(uint8_t strategy);
};



#endif /* MONITORINGLISTADAPTER_H_ */
