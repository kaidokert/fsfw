/*
 * MonitoringHelper.h
 *
 *  Created on: 07.07.2014
 *      Author: baetz
 */

#ifndef MONITORINGHELPER_H_
#define MONITORINGHELPER_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/monitoring/HasMonitorsIF.h>
#include <framework/storagemanager/StorageManagerIF.h>

class MonitoringHelper {
public:
	MonitoringHelper(HasMonitorsIF* limitOwner);
	virtual ~MonitoringHelper();
	ReturnValue_t handleMessage(CommandMessage* message);
	ReturnValue_t initialize();
private:
	HasMonitorsIF* owner;
	StorageManagerIF* ipcStore; //might be static
	ReturnValue_t handleReportingStrategyMessage(uint8_t strategy, store_address_t storeId);
	ReturnValue_t handleUpdateParameterMonitor(store_address_t storeId);
	ReturnValue_t handleUpdateObjectMonitor(store_address_t storeId);
	void replyReturnValue(CommandMessage* message, ReturnValue_t result);
};

#endif /* MONITORINGHELPER_H_ */
