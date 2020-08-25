#include "ControllerSet.h"

ControllerSet::ControllerSet() {

}

ControllerSet::~ControllerSet() {
}

void ControllerSet::setInvalid() {
	read();
	setToDefault();
	commit(PoolVariableIF::INVALID);
}
