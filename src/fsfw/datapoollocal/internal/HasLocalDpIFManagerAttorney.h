#ifndef FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_

#include "fsfw/datapoollocal/localPoolDefinitions.h"

class HasLocalDataPoolIF;
class LocalPoolDataSetBase;
class LocalPoolObjectBase;

class HasLocalDpIFManagerAttorney {
  static LocalPoolDataSetBase* getDataSetHandle(HasLocalDataPoolIF* clientIF, sid_t sid);

  static LocalPoolObjectBase* getPoolObjectHandle(HasLocalDataPoolIF* clientIF,
                                                  lp_id_t localPoolId);

  static object_id_t getObjectId(HasLocalDataPoolIF* clientIF);

  friend class LocalDataPoolManager;
};

#endif /* FSFW_DATAPOOLLOCAL_HASLOCALDPIFMANAGERATTORNEY_H_ */
