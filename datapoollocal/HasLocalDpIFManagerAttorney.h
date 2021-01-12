#ifndef FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_

#include "HasLocalDataPoolIF.h"

class LocalPoolDataSetBase;

class HasLocalDpIFManagerAttorney {

	static LocalPoolDataSetBase* getDataSetHandle(HasLocalDataPoolIF* interface, sid_t sid) {
		return interface->getDataSetHandle(sid);
	}

	static LocalPoolObjectBase* getPoolObjectHandle(HasLocalDataPoolIF* interface,
			lp_id_t localPoolId) {
		return interface->getPoolObjectHandle(localPoolId);
	}

	static object_id_t getObjectId(HasLocalDataPoolIF* interface) {
		return interface->getObjectId();
	}

	friend class LocalDataPoolManager;
};

#endif /* FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_ */
