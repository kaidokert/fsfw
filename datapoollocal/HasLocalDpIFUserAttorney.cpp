#include "HasLocalDpIFUserAttorney.h"
#include "HasLocalDataPoolIF.h"

AccessPoolManagerIF* HasLocalDpIFUserAttorney::getAccessorHandle(HasLocalDataPoolIF *interface) {
	return interface->getAccessorHandle();
}
