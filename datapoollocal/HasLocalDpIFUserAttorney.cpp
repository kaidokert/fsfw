#include "HasLocalDpIFUserAttorney.h"
#include "AccessLocalPoolF.h"
#include "HasLocalDataPoolIF.h"

AccessPoolManagerIF* HasLocalDpIFUserAttorney::getAccessorHandle(HasLocalDataPoolIF *clientIF) {
	return clientIF->getAccessorHandle();
}
