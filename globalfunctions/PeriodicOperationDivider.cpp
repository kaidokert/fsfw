#include "PeriodicOperationDivider.h"


PeriodicOperationDivider::PeriodicOperationDivider(uint32_t divider,
		bool resetAutomatically): resetAutomatically(resetAutomatically),
		counter(divider), divider(divider) {
}

bool PeriodicOperationDivider::checkAndIncrement() {
	if(counter >= divider) {
		if(resetAutomatically) {
			counter = 0;
		}
		return true;
	}
	counter ++;
	return false;
}

void PeriodicOperationDivider::resetCounter() {
	counter = 0;
}

void PeriodicOperationDivider::setDivider(uint32_t newDivider) {
	divider = newDivider;
}

uint32_t PeriodicOperationDivider::getCounter() const {
	return counter;
}

uint32_t PeriodicOperationDivider::getDivider() const {
	return divider;
}
