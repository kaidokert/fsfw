#include "fsfw/power/Fuse.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/monitoring/LimitViolationReporter.h"
#include "fsfw/monitoring/MonitoringMessageContent.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/SerialFixedArrayListAdapter.h"

object_id_t Fuse::powerSwitchId = 0;

Fuse::Fuse(object_id_t fuseObjectId, uint8_t fuseId, sid_t variableSet, VariableIds ids,
           float maxCurrent, uint16_t confirmationCount)
    : SystemObject(fuseObjectId),
      oldFuseState(0),
      fuseId(fuseId),
      currentLimit(fuseObjectId, 1, ids.pidCurrent, confirmationCount, maxCurrent,
                   FUSE_CURRENT_HIGH),
      powerMonitor(fuseObjectId, 2, ids.poolIdPower, confirmationCount),
      set(variableSet),
      voltage(ids.pidVoltage, &set),
      current(ids.pidCurrent, &set),
      state(ids.pidState, &set),
      power(ids.poolIdPower, &set, PoolVariableIF::VAR_READ_WRITE),
      parameterHelper(this),
      healthHelper(this, fuseObjectId) {
  commandQueue = QueueFactory::instance()->createMessageQueue();
}

Fuse::~Fuse() { QueueFactory::instance()->deleteMessageQueue(commandQueue); }

void Fuse::addDevice(PowerComponentIF* switchSet) { devices.push_back(switchSet); }

ReturnValue_t Fuse::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = parameterHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = healthHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  powerIF = ObjectManager::instance()->get<PowerSwitchIF>(powerSwitchId);
  if (powerIF == NULL) {
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

void Fuse::calculatePowerLimits(float* low, float* high) {
  for (DeviceList::iterator iter = devices.begin(); iter != devices.end(); iter++) {
    if (areSwitchesOfComponentOn(iter)) {
      *low += (*iter)->getMin();
      *high += (*iter)->getMax();
    }
  }
}

ReturnValue_t Fuse::check() {
  set.read();
  if (!healthHelper.healthTable->isHealthy(getObjectId())) {
    setAllMonitorsToUnchecked();
    set.setValidity(false, true);
    return set.commit();
  }
  ReturnValue_t result = returnvalue::OK;
  checkFuseState();
  calculateFusePower();
  // Check if power is valid and if fuse state is off or invalid.
  if (!power.isValid() || (state == 0) || !state.isValid()) {
    result = powerMonitor.setToInvalid();
  } else {
    float lowLimit = 0.0;
    float highLimit = RESIDUAL_POWER;
    calculatePowerLimits(&lowLimit, &highLimit);
    result = powerMonitor.checkPower(power.value, lowLimit, highLimit);
    if (result == MonitoringIF::BELOW_LOW_LIMIT) {
      reportEvents(POWER_BELOW_LOW_LIMIT);
    } else if (result == MonitoringIF::ABOVE_HIGH_LIMIT) {
      reportEvents(POWER_ABOVE_HIGH_LIMIT);
    }
  }
  set.commit();
  return result;
}

ReturnValue_t Fuse::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                              Endianness streamEndianness) const {
  ReturnValue_t result = returnvalue::FAILED;
  for (DeviceList::const_iterator iter = devices.begin(); iter != devices.end(); iter++) {
    result = (*iter)->serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  return returnvalue::OK;
}

size_t Fuse::getSerializedSize() const {
  uint32_t size = 0;
  for (DeviceList::const_iterator iter = devices.begin(); iter != devices.end(); iter++) {
    size += (*iter)->getSerializedSize();
  }
  return size;
}

ReturnValue_t Fuse::deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
  ReturnValue_t result = returnvalue::FAILED;
  for (DeviceList::iterator iter = devices.begin(); iter != devices.end(); iter++) {
    result = (*iter)->deSerialize(buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  return returnvalue::OK;
}

uint8_t Fuse::getFuseId() const { return fuseId; }

void Fuse::calculateFusePower() {
  ReturnValue_t result1 = currentLimit.check();
  if (result1 != returnvalue::OK || !(voltage.isValid())) {
    power.setValid(PoolVariableIF::INVALID);
    return;
  }
  // Calculate fuse power.
  power.value = current.value * voltage.value;
  power.setValid(PoolVariableIF::VALID);
}

ReturnValue_t Fuse::performOperation(uint8_t opCode) {
  checkCommandQueue();
  return returnvalue::OK;
}

void Fuse::reportEvents(Event event) {
  if (!powerMonitor.isEventEnabled()) {
    return;
  }
  for (DeviceList::iterator iter = devices.begin(); iter != devices.end(); iter++) {
    if (areSwitchesOfComponentOn(iter)) {
      EventManagerIF::triggerEvent((*iter)->getDeviceObjectId(), event);
    }
  }
}

MessageQueueId_t Fuse::getCommandQueue() const { return commandQueue->getId(); }

void Fuse::setAllMonitorsToUnchecked() {
  currentLimit.setToUnchecked();
  powerMonitor.setToUnchecked();
}

void Fuse::checkCommandQueue() {
  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result != returnvalue::OK) {
    return;
  }
  result = healthHelper.handleHealthCommand(&command);
  if (result == returnvalue::OK) {
    return;
  }
  result = parameterHelper.handleParameterMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }
  command.setToUnknownCommand();
  commandQueue->reply(&command);
}

void Fuse::checkFuseState() {
  if (!state.isValid()) {
    oldFuseState = 0;
    return;
  }
  if (state == 0) {
    if (oldFuseState != 0) {
      reportEvents(FUSE_WENT_OFF);
    }
  }
  oldFuseState = state.value;
}

float Fuse::getPower() {
  if (power.isValid()) {
    return power.value;
  } else {
    return 0.0;
  }
}

void Fuse::setDataPoolEntriesInvalid() {
  set.read();
  set.setValidity(false, true);
  set.commit();
}

ReturnValue_t Fuse::getParameter(uint8_t domainId, uint8_t uniqueId,
                                 ParameterWrapper* parameterWrapper,
                                 const ParameterWrapper* newValues, uint16_t startAtIndex) {
  ReturnValue_t result =
      currentLimit.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  result = powerMonitor.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  return result;
}

bool Fuse::areSwitchesOfComponentOn(DeviceList::iterator iter) {
  if (powerIF->getSwitchState((*iter)->getSwitchId1()) != PowerSwitchIF::SWITCH_ON) {
    return false;
  }
  if ((*iter)->hasTwoSwitches()) {
    if ((powerIF->getSwitchState((*iter)->getSwitchId2()) != PowerSwitchIF::SWITCH_ON)) {
      return false;
    }
  }
  return true;
}

bool Fuse::isPowerValid() { return power.isValid(); }

ReturnValue_t Fuse::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return returnvalue::OK;
}

HasHealthIF::HealthState Fuse::getHealth() { return healthHelper.getHealth(); }

ReturnValue_t Fuse::PowerMonitor::checkPower(float sample, float lowerLimit, float upperLimit) {
  if (sample > upperLimit) {
    return this->monitorStateIs(MonitoringIF::ABOVE_HIGH_LIMIT, sample, upperLimit);
  } else if (sample < lowerLimit) {
    return this->monitorStateIs(MonitoringIF::BELOW_LOW_LIMIT, sample, lowerLimit);
  } else {
    return this->monitorStateIs(returnvalue::OK, sample, 0.0);  // Within limits.
  }
}
