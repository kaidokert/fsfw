#include "HasLocalDpIFManagerAttorney.h"
#include "LocalPoolObjectBase.h"
#include "LocalPoolDataSetBase.h"
#include "HasLocalDataPoolIF.h"

LocalPoolDataSetBase* HasLocalDpIFManagerAttorney::getDataSetHandle(HasLocalDataPoolIF* clientIF,
		sid_t sid) {
	return clientIF->getDataSetHandle(sid);
}

LocalPoolObjectBase* HasLocalDpIFManagerAttorney::getPoolObjectHandle(HasLocalDataPoolIF* clientIF,
		lp_id_t localPoolId) {
	return clientIF->getPoolObjectHandle(localPoolId);
}

object_id_t HasLocalDpIFManagerAttorney::getObjectId(HasLocalDataPoolIF* clientIF) {
	return clientIF->getObjectId();
}
