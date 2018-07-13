#include "Timekeeper.h"
#include <FreeRTOSConfig.h>

Timekeeper::Timekeeper() :
		offset( { 0, 0 }) {
	// TODO Auto-generated constructor stub

}

Timekeeper * Timekeeper::myinstance = NULL;

const timeval& Timekeeper::getOffset() const {
	return offset;
}

Timekeeper* Timekeeper::instance() {
	if (myinstance == NULL) {
		myinstance = new Timekeeper();
	}
	return myinstance;
}

void Timekeeper::setOffset(const timeval& offset) {
	this->offset = offset;
}

Timekeeper::~Timekeeper() {
	// TODO Auto-generated destructor stub
}

timeval Timekeeper::ticksToTimeval(TickType_t ticks) {
	timeval uptime;
	uptime.tv_sec = ticks / configTICK_RATE_HZ;

	//TODO explain, think about overflow
	uint32_t subsecondTicks = ticks % configTICK_RATE_HZ;
	uint64_t usecondTicks = subsecondTicks * 1000000;

	uptime.tv_usec = usecondTicks / configTICK_RATE_HZ;

	return uptime;
}
