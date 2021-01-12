#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETATTORNEY_H_

#include "../LocalPoolDataSetBase.h"

class LocalPoolDataSetAttorney {
private:
	static void setDiagnostic(LocalPoolDataSetBase& set, bool diagnostics) {
		set.setDiagnostic(diagnostics);
	}

	static bool isDiagnostics(LocalPoolDataSetBase& set) {
		return set.isDiagnostics();
	}

	static void initializePeriodicHelper(LocalPoolDataSetBase& set, float collectionInterval,
			uint32_t minimumPeriodicIntervalMs,
			bool isDiagnostics, uint8_t nonDiagIntervalFactor = 5) {
		set.initializePeriodicHelper(collectionInterval, minimumPeriodicIntervalMs, isDiagnostics,
				nonDiagIntervalFactor);
	}

	static void setReportingEnabled(LocalPoolDataSetBase& set, bool enabled) {
		set.setReportingEnabled(enabled);
	}

	static bool getReportingEnabled(LocalPoolDataSetBase& set) {
		return set.getReportingEnabled();
	}

	static PeriodicHousekeepingHelper* getPeriodicHelper(LocalPoolDataSetBase& set) {
		return set.periodicHelper;
	}

	friend class LocalDataPoolManager;
};


#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETATTORNEY_H_ */
