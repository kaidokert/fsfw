#include "HasLocalDpIFManagerAttorney.h"
#include "LocalPoolObjectBase.h"
#include "LocalPoolDataSetBase.h"
#include "HasLocalDataPoolIF.h"

LocalPoolDataSetBase* HasLocalDpIFManagerAttorney::getDataSetHandle(HasLocalDataPoolIF* interface,
		sid_t sid) {
	return interface->getDataSetHandle(sid);
}

LocalPoolObjectBase* HasLocalDpIFManagerAttorney::getPoolObjectHandle(HasLocalDataPoolIF* interface,
		lp_id_t localPoolId) {
	return interface->getPoolObjectHandle(localPoolId);
}

object_id_t HasLocalDpIFManagerAttorney::getObjectId(HasLocalDataPoolIF* interface) {
	return interface->getObjectId();
}
