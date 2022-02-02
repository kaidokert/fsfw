#include "HasLocalDpIFUserAttorney.h"

#include "fsfw/datapoollocal/AccessLocalPoolF.h"
#include "fsfw/datapoollocal/HasLocalDataPoolIF.h"

AccessPoolManagerIF* HasLocalDpIFUserAttorney::getAccessorHandle(HasLocalDataPoolIF* clientIF) {
  return clientIF->getAccessorHandle();
}
