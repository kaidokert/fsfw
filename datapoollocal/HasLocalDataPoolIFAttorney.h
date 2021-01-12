#ifndef FSFW_DATAPOOLLOCAL_HASLOCALDATAPOOLIFATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_HASLOCALDATAPOOLIFATTORNEY_H_

#include <fsfw/datapoollocal/LocalDataPoolManager.h>

class HasLocalDpIFUserAttorney {
private:

	static AccessPoolManagerIF* getAccessorHandle(HasLocalDataPoolIF* interface) {
		return interface->getAccessorHandle();
	}

	friend class LocalPoolObjectBase;
	friend class LocalPoolDataSetBase;

};

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



#endif /* FSFW_DATAPOOLLOCAL_HASLOCALDATAPOOLIFATTORNEY_H_ */
