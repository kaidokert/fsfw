#include "fsfw/devicehandlers/DeviceHandlerFailureIsolation.h"

#include "fsfw/devicehandlers/DeviceHandlerIF.h"
#include "fsfw/health/HealthTableIF.h"
#include "fsfw/modes/HasModesIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/power/Fuse.h"
#include "fsfw/serviceinterface/ServiceInterfaceStream.h"
#include "fsfw/thermal/ThermalComponentIF.h"

object_id_t DeviceHandlerFailureIsolation::powerConfirmationId = objects::NO_OBJECT;

DeviceHandlerFailureIsolation::DeviceHandlerFailureIsolation(object_id_t owner, object_id_t parent)
    : FailureIsolationBase(owner, parent),
      strangeReplyCount(DEFAULT_MAX_STRANGE_REPLIES, DEFAULT_STRANGE_REPLIES_TIME_MS,
                        parameterDomainBase++),
      missedReplyCount(DEFAULT_MAX_MISSED_REPLY_COUNT, DEFAULT_MISSED_REPLY_TIME_MS,
                       parameterDomainBase++),
      recoveryCounter(DEFAULT_MAX_REBOOT, DEFAULT_REBOOT_TIME_MS, parameterDomainBase++),
      fdirState(NONE) {}

DeviceHandlerFailureIsolation::~DeviceHandlerFailureIsolation() {}

ReturnValue_t DeviceHandlerFailureIsolation::eventReceived(EventMessage* event) {
  if (isFdirInActionOrAreWeFaulty(event)) {
    return returnvalue::OK;
  }
  ReturnValue_t result = returnvalue::FAILED;
  switch (event->getEvent()) {
    case HasModesIF::MODE_TRANSITION_FAILED:
    case HasModesIF::OBJECT_IN_INVALID_MODE:
      // We'll try a recovery as long as defined in MAX_REBOOT.
      // Might cause some AssemblyBase cycles, so keep number low.
      handleRecovery(event->getEvent());
      break;
    case DeviceHandlerIF::DEVICE_INTERPRETING_REPLY_FAILED:
    case DeviceHandlerIF::DEVICE_READING_REPLY_FAILED:
    case DeviceHandlerIF::DEVICE_UNREQUESTED_REPLY:
    case DeviceHandlerIF::DEVICE_UNKNOWN_REPLY:  // Some DH's generate generic reply-ids.
    case DeviceHandlerIF::DEVICE_BUILDING_COMMAND_FAILED:
      // These faults all mean that there were stupid replies from a device.
      if (strangeReplyCount.incrementAndCheck()) {
        handleRecovery(event->getEvent());
      }
      break;
    case DeviceHandlerIF::DEVICE_SENDING_COMMAND_FAILED:
    case DeviceHandlerIF::DEVICE_REQUESTING_REPLY_FAILED:
      // The two above should never be confirmed.
    case DeviceHandlerIF::DEVICE_MISSED_REPLY:
      result = sendConfirmationRequest(event);
      if (result == returnvalue::OK) {
        break;
      }
      // else
      if (missedReplyCount.incrementAndCheck()) {
        handleRecovery(event->getEvent());
      }
      break;
    case StorageManagerIF::GET_DATA_FAILED:
    case StorageManagerIF::STORE_DATA_FAILED:
      // Rather strange bugs, occur in RAW mode only. Ignore.
      break;
    case DeviceHandlerIF::INVALID_DEVICE_COMMAND:
      // Ignore, is bad configuration. We can't do anything in flight.
      break;
    case HasHealthIF::HEALTH_INFO:
    case HasModesIF::MODE_INFO:
    case HasModesIF::CHANGING_MODE:
      // Do nothing, but mark as handled.
      break;
      //****Power*****
    case PowerSwitchIF::SWITCH_WENT_OFF:
      if (powerConfirmation != MessageQueueIF::NO_QUEUE) {
        result = sendConfirmationRequest(event, powerConfirmation);
        if (result == returnvalue::OK) {
          setFdirState(DEVICE_MIGHT_BE_OFF);
        }
      }
      break;
    case Fuse::FUSE_WENT_OFF:
      // Not so good, because PCDU reacted.
    case Fuse::POWER_ABOVE_HIGH_LIMIT:
      // Better, because software detected over-current.
      setFaulty(event->getEvent());
      break;
    case Fuse::POWER_BELOW_LOW_LIMIT:
      // Device might got stuck during boot, retry.
      handleRecovery(event->getEvent());
      break;
      //****Thermal*****
    case ThermalComponentIF::COMPONENT_TEMP_LOW:
    case ThermalComponentIF::COMPONENT_TEMP_HIGH:
    case ThermalComponentIF::COMPONENT_TEMP_OOL_LOW:
    case ThermalComponentIF::COMPONENT_TEMP_OOL_HIGH:
      // Well, the device is not really faulty, but it is required to stay off as long as possible.
      setFaulty(event->getEvent());
      break;
    case ThermalComponentIF::TEMP_NOT_IN_OP_RANGE:
      // Ignore, is information only.
      break;
      //*******Default monitoring variables. Are currently not used.*****
      //	case DeviceHandlerIF::MONITORING_LIMIT_EXCEEDED:
      //		setFaulty(event->getEvent());
      //		break;
      //	case DeviceHandlerIF::MONITORING_AMBIGUOUS:
      //		break;
    default:
      // We don't know the event, someone else should handle it.
      return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

void DeviceHandlerFailureIsolation::eventConfirmed(EventMessage* event) {
  switch (event->getEvent()) {
    case DeviceHandlerIF::DEVICE_SENDING_COMMAND_FAILED:
    case DeviceHandlerIF::DEVICE_REQUESTING_REPLY_FAILED:
    case DeviceHandlerIF::DEVICE_MISSED_REPLY:
      if (missedReplyCount.incrementAndCheck()) {
        handleRecovery(event->getEvent());
      }
      break;
    case PowerSwitchIF::SWITCH_WENT_OFF:
      // This means the switch went off only for one device.
      handleRecovery(event->getEvent());
      break;
    default:
      break;
  }
}

void DeviceHandlerFailureIsolation::decrementFaultCounters() {
  strangeReplyCount.checkForDecrement();
  missedReplyCount.checkForDecrement();
  recoveryCounter.checkForDecrement();
}

void DeviceHandlerFailureIsolation::handleRecovery(Event reason) {
  clearFaultCounters();
  if (not recoveryCounter.incrementAndCheck()) {
    startRecovery(reason);
  } else {
    setFaulty(reason);
  }
}

void DeviceHandlerFailureIsolation::wasParentsFault(EventMessage* event) {
  // We'll better ignore the SWITCH_WENT_OFF event and await a system-wide reset.
  // This means, no fault message will come through until a MODE_ or
  // HEALTH_INFO message comes through -> Is that ok?
  // Same issue in TxFailureIsolation!
  //	if ((event->getEvent() == PowerSwitchIF::SWITCH_WENT_OFF)
  //			&& (fdirState != RECOVERY_ONGOING)) {
  //		setFdirState(NONE);
  //	}
}

void DeviceHandlerFailureIsolation::clearFaultCounters() {
  strangeReplyCount.clear();
  missedReplyCount.clear();
}

ReturnValue_t DeviceHandlerFailureIsolation::initialize() {
  ReturnValue_t result = FailureIsolationBase::initialize();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DeviceHandlerFailureIsolation::initialize: Could not"
                  " initialize FailureIsolationBase."
               << std::endl;
#endif
    return result;
  }
  ConfirmsFailuresIF* power =
      ObjectManager::instance()->get<ConfirmsFailuresIF>(powerConfirmationId);
  if (power != nullptr) {
    powerConfirmation = power->getEventReceptionQueue();
  }

  return returnvalue::OK;
}

void DeviceHandlerFailureIsolation::setFdirState(FDIRState state) {
  FailureIsolationBase::throwFdirEvent(FDIR_CHANGED_STATE, state, fdirState);
  fdirState = state;
}

void DeviceHandlerFailureIsolation::triggerEvent(Event event, uint32_t parameter1,
                                                 uint32_t parameter2) {
  // Do not throw error events if fdirState != none.
  // This will still forward MODE and HEALTH INFO events in any case.
  if (fdirState == NONE || event::getSeverity(event) == severity::INFO) {
    FailureIsolationBase::triggerEvent(event, parameter1, parameter2);
  }
}

bool DeviceHandlerFailureIsolation::isFdirActionInProgress() { return (fdirState != NONE); }

void DeviceHandlerFailureIsolation::startRecovery(Event reason) {
  throwFdirEvent(FDIR_STARTS_RECOVERY, event::getEventId(reason));
  setOwnerHealth(HasHealthIF::NEEDS_RECOVERY);
  setFdirState(RECOVERY_ONGOING);
}

ReturnValue_t DeviceHandlerFailureIsolation::getParameter(uint8_t domainId, uint8_t uniqueId,
                                                          ParameterWrapper* parameterWrapper,
                                                          const ParameterWrapper* newValues,
                                                          uint16_t startAtIndex) {
  ReturnValue_t result =
      strangeReplyCount.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  result =
      missedReplyCount.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  result =
      recoveryCounter.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  return INVALID_DOMAIN_ID;
}

void DeviceHandlerFailureIsolation::setFaulty(Event reason) {
  throwFdirEvent(FDIR_TURNS_OFF_DEVICE, event::getEventId(reason));
  setOwnerHealth(HasHealthIF::FAULTY);
  setFdirState(AWAIT_SHUTDOWN);
}

bool DeviceHandlerFailureIsolation::isFdirInActionOrAreWeFaulty(EventMessage* event) {
  if (fdirState != NONE) {
    // Only wait for those events, ignore all others.
    if (event->getParameter1() == HasHealthIF::HEALTHY &&
        event->getEvent() == HasHealthIF::HEALTH_INFO) {
      setFdirState(NONE);
    }
    if (event->getEvent() == HasModesIF::MODE_INFO && fdirState != RECOVERY_ONGOING) {
      setFdirState(NONE);
    }
    return true;
  }

  if (owner == nullptr) {
    // Configuration error.
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DeviceHandlerFailureIsolation::"
               << "isFdirInActionOrAreWeFaulty: Owner not set!" << std::endl;
#endif
    return false;
  }

  if (owner->getHealth() == HasHealthIF::FAULTY ||
      owner->getHealth() == HasHealthIF::PERMANENT_FAULTY) {
    // Ignore all events in case device is already faulty.
    return true;
  }
  return false;
}
