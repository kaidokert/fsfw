#include "fsfw/pus/Service9TimeManagement.h"

#include "fsfw/events/EventManagerIF.h"
#include "fsfw/pus/servicepackets/Service9Packets.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/timemanager/CCSDSTime.h"

Service9TimeManagement::Service9TimeManagement(PsbParams params) : PusServiceBase(params) {
  params.name = "PUS 9 Time MGMT";
}

Service9TimeManagement::~Service9TimeManagement() = default;

ReturnValue_t Service9TimeManagement::performService() { return returnvalue::OK; }

ReturnValue_t Service9TimeManagement::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case SUBSERVICE::SET_TIME: {
      return setTime();
    }
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service9TimeManagement::setTime() {
  Clock::TimeOfDay_t timeToSet;
  TimePacket timePacket(currentPacket.getUserData(), currentPacket.getUserDataLen());
  ReturnValue_t result =
      CCSDSTime::convertFromCcsds(&timeToSet, timePacket.getTime(), timePacket.getTimeSize());
  if (result != returnvalue::OK) {
    triggerEvent(CLOCK_SET_FAILURE, result, 0);
    return result;
  }

  uint32_t formerUptime;
  Clock::getUptime(&formerUptime);
  result = Clock::setClock(&timeToSet);

  if (result == returnvalue::OK) {
    uint32_t newUptime;
    Clock::getUptime(&newUptime);
    triggerEvent(CLOCK_SET, newUptime, formerUptime);
    return returnvalue::OK;
  } else {
    triggerEvent(CLOCK_SET_FAILURE, result, 0);
    return returnvalue::FAILED;
  }
}
