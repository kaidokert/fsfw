#include "fsfw/osal/freertos/Timekeeper.h"

#include "FreeRTOSConfig.h"

Timekeeper* Timekeeper::myinstance = nullptr;

Timekeeper::Timekeeper() : offset({0, 0}) {}

Timekeeper::~Timekeeper() {}

const timeval& Timekeeper::getOffset() const { return offset; }

Timekeeper* Timekeeper::instance() {
  if (myinstance == nullptr) {
    myinstance = new Timekeeper();
  }
  return myinstance;
}

void Timekeeper::setOffset(const timeval& offset) { this->offset = offset; }

timeval Timekeeper::ticksToTimeval(TickType_t ticks) {
  timeval uptime;
  uptime.tv_sec = ticks / configTICK_RATE_HZ;

  // TODO explain, think about overflow
  uint32_t subsecondTicks = ticks % configTICK_RATE_HZ;
  uint64_t usecondTicks = subsecondTicks * 1000000;

  uptime.tv_usec = usecondTicks / configTICK_RATE_HZ;

  return uptime;
}

TickType_t Timekeeper::getTicks() { return xTaskGetTickCount(); }
