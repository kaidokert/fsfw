#include "fsfw/globalfunctions/PeriodicOperationDivider.h"

PeriodicOperationDivider::PeriodicOperationDivider(uint32_t divider, bool resetAutomatically)
    : resetAutomatically(resetAutomatically), divider(divider) {}

bool PeriodicOperationDivider::checkAndIncrement() {
  bool opNecessary = check();
  if (opNecessary and resetAutomatically) {
    resetCounter();
  } else {
    counter++;
  }
  return opNecessary;
}

bool PeriodicOperationDivider::check() {
  if (counter >= divider) {
    return true;
  }
  return false;
}

void PeriodicOperationDivider::resetCounter() { counter = 1; }

void PeriodicOperationDivider::setDivider(uint32_t newDivider) { divider = newDivider; }

uint32_t PeriodicOperationDivider::getCounter() const { return counter; }

uint32_t PeriodicOperationDivider::getDivider() const { return divider; }
